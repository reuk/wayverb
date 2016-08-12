#include "combined/engine.h"

#include "common/aligned/set.h"
#include "common/cl_common.h"
#include "common/config.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/map_to_vector.h"
#include "common/receiver_settings.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "raytracer/attenuator.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "waveguide/attenuator/hrtf.h"
#include "waveguide/attenuator/microphone.h"
#include "waveguide/default_kernel.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/mesh/model.h"
#include "waveguide/mesh/setup.h"
#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/postprocessor/visualiser.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/waveguide.h"

#include <cmath>

namespace {

/*

/// courant number is 1 / sqrt(3) for a rectilinear mesh
/// but sqrt isn't constexpr >:(
constexpr auto COURANT = 0.577350269;

/// given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2
double distance_for_unit_intensity(double strength) {
    return std::sqrt(strength / (4 * M_PI));
}

/// r = distance at which the geometric sound source has intensity 1W/m^2
/// sr = waveguide mesh sampling rate
constexpr double rectilinear_calibration_factor(double r, double sr) {
    auto x = SPEED_OF_SOUND / (COURANT * sr);
    return r / (x * 0.3405);
}

*/

class intermediate_impl : public wayverb::intermediate {
public:
    intermediate_impl(
            const glm::vec3& source,
            const glm::vec3& receiver,
            double waveguide_sample_rate,
            raytracer::results&& raytracer_results,
            aligned::vector<waveguide::run_step_output>&& waveguide_results)
            : source(source)
            , receiver(receiver)
            , waveguide_sample_rate(waveguide_sample_rate)
            , raytracer_results(std::move(raytracer_results))
            , waveguide_results(std::move(waveguide_results)) {}

    aligned::vector<aligned::vector<float>> attenuate(
            const compute_context& cc,
            const model::ReceiverSettings& receiver,
            double output_sample_rate,
            const state_callback& callback) const override {
        callback(wayverb::state::postprocessing, 1.0);

        //  attenuate raytracer results
        auto raytracer_output = raytracer::run_attenuation(
                cc, receiver, raytracer_results, output_sample_rate);
        //  attenuate waveguide results
        auto waveguide_output = waveguide::run_attenuation(
                receiver, waveguide_results, waveguide_sample_rate);

        //  correct raytracer results for dc
        filter::extra_linear_dc_blocker blocker;
        for (auto& i : raytracer_output) {
            i = filter::run_two_pass(blocker, i.begin(), i.end());
        }

        //  correct waveguide results for dc
        for (auto& i : waveguide_output) {
            i = filter::run_two_pass(blocker, i.begin(), i.end());
        }

        //  correct waveguide sampling rate
        for (auto& i : waveguide_output) {
            i = waveguide::adjust_sampling_rate(
                    std::move(i), waveguide_sample_rate, output_sample_rate);
        }

        //  TODO scale waveguide output to match raytracer level
        //  TODO crossover filtering

        //  mixdown
        assert(raytracer_output.size() == waveguide_output.size());
        auto mm = std::minmax(raytracer_output,
                              waveguide_output,
                              [](const auto& a, const auto& b) {
                                  return a.front().size() < b.front().size();
                              });

        auto& min = mm.first;
        auto& max = mm.second;

        aligned::vector<aligned::vector<float>> ret;
        ret.reserve(max.size());
        proc::transform(
                max,
                min.begin(),
                std::back_inserter(ret),
                [](const auto& a, const auto& b) {
                    auto ret = a;
                    proc::transform(
                            b, a.begin(), ret.begin(), [](auto a, auto b) {
                                return a + b;
                            });
                    return ret;
                });

        return max;
    }

private:
    glm::vec3 source;
    glm::vec3 receiver;
    double waveguide_sample_rate;

    raytracer::results raytracer_results;
    aligned::vector<waveguide::run_step_output> waveguide_results;
};

float max_reflectivity(const volume_type& vt) {
    return *proc::max_element(vt.s);
}

float max_reflectivity(const surface& surface) {
    return std::max(max_reflectivity(surface.diffuse),
                    max_reflectivity(surface.specular));
}

float max_reflectivity(const copyable_scene_data::material& material) {
    return max_reflectivity(material.surface);
}

float max_reflectivity(
        const aligned::vector<copyable_scene_data::material>& materials) {
    return std::accumulate(materials.begin() + 1,
                           materials.end(),
                           max_reflectivity(materials.front()),
                           [](const auto& i, const auto& j) {
                               return std::max(i, max_reflectivity(j));
                           });
}

}  // namespace

namespace wayverb {

class engine::impl final {
public:
    impl(const compute_context& cc,
         const copyable_scene_data& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays)
            : impl(cc,
                   scene_data,
                   source,
                   receiver,
                   waveguide_sample_rate,
                   rays,
                   raytracer::compute_optimum_reflection_number(
                           decibels::db2a(-48.0),
                           max_reflectivity(scene_data.get_materials()))) {}

    impl(const compute_context& cc,
         const copyable_scene_data& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays,
         size_t impulses)
            : compute_context(cc)
            , mesh_spacing(waveguide::config::grid_spacing(
                      speed_of_sound, 1 / waveguide_sample_rate))
            , voxelised(scene_data,
                        5,
                        waveguide::compute_adjusted_boundary(
                                scene_data.get_aabb(), receiver, mesh_spacing))
            , model(waveguide::mesh::compute_model(cc.get_context(),
                                                   cc.get_device(),
                                                   voxelised,
                                                   mesh_spacing))
            , source(source)
            , receiver(receiver)
            , waveguide_sample_rate(waveguide_sample_rate)
            , rays(rays)
            , impulses(impulses)
            , source_index(compute_index(model.get_descriptor(), source))
            , receiver_index(compute_index(model.get_descriptor(), receiver)) {
        const auto is_index_inside = [&](auto index) {
            return waveguide::mesh::is_inside(
                    model.get_structure().get_condensed_nodes()[index]);
        };

        if (!is_index_inside(source_index)) {
            throw std::runtime_error(
                    "invalid source position - probably outside mesh");
        }
        if (!is_index_inside(receiver_index)) {
            throw std::runtime_error(
                    "invalid receiver position - probably outside mesh");
        }
    }

    std::unique_ptr<intermediate> run(std::atomic_bool& keep_going,
                                      const state_callback& callback) {
        //  RAYTRACER  -------------------------------------------------------//
        callback(state::starting_raytracer, 1.0);
        auto raytracer_results = raytracer::run(
                compute_context.get_context(),
                compute_context.get_device(),
                voxelised,
                source,
                receiver,
                rays,
                impulses,
                std::min(size_t{10},
                         impulses),  //  TODO set this more intelligently
                keep_going,
                [&](auto step) {
                    callback(state::running_raytracer, step / (impulses - 1.0));
                });

        if (!(keep_going && raytracer_results)) {
            return nullptr;
        }

        callback(state::finishing_raytracer, 1.0);

        if (raytracer_visual_callback) {
            raytracer_visual_callback(
                    raytracer_results->get_diffuse(), source, receiver);
        }

        const auto impulses = raytracer_results->get_impulses();

        //  look for the max time of an impulse
        const auto max_time =
                proc::max_element(impulses, [](const auto& a, const auto& b) {
                    return a.time < b.time;
                })->time;

        //  WAVEGUIDE  -------------------------------------------------------//
        callback(state::starting_waveguide, 1.0);

        const auto input = waveguide::default_kernel(waveguide_sample_rate);

        //  this is the number of steps to run the raytracer for
        //  TODO is there a less dumb way of doing this?
        const auto steps = std::ceil(max_time * waveguide_sample_rate) +
                           input.opaque_kernel_size;

        waveguide::preprocessor::single_soft_source preprocessor(source_index,
                                                                 input.kernel);

        //  If the max raytracer time is large this could take forever...
        aligned::vector<waveguide::run_step_output> waveguide_results;
        waveguide_results.reserve(steps);
        aligned::vector<waveguide::step_postprocessor> postprocessors{
                waveguide::postprocessor::microphone(
                        model.get_descriptor(),
                        receiver_index,
                        waveguide_sample_rate,
                        [&waveguide_results](const auto& i) {
                            waveguide_results.push_back(i);
                        })};

        if (waveguide_visual_callback) {
            postprocessors.push_back(waveguide_visual_callback);
        }

        const auto waveguide_steps_completed = waveguide::run(
                compute_context.get_context(),
                compute_context.get_device(),
                model,
                steps,
                preprocessor,
                postprocessors,
                [&](auto step) {
                    callback(state::running_waveguide, step / (steps - 1.0));
                },
                keep_going);

        if (waveguide_steps_completed != steps) {
            return nullptr;
        }

        //  if we got here, we can assume waveguide_results is complete and
        //  valid

        callback(state::finishing_waveguide, 1.0);

        //  correct for filter time offset
        assert(input.correction_offset_in_samples < waveguide_results.size());

        waveguide_results.erase(
                waveguide_results.begin(),
                waveguide_results.begin() + input.correction_offset_in_samples);

        std::unique_ptr<intermediate> ret = std::make_unique<intermediate_impl>(
                source,
                receiver,
                waveguide_sample_rate,
                std::move(*raytracer_results),
                std::move(waveguide_results));

        return ret;
    }

    aligned::vector<glm::vec3> get_node_positions() const {
        return model.get_node_positions();
    }

    void register_waveguide_visual_callback(
            waveguide_visual_callback_t callback) {
        //  visualiser returns current waveguide step, but we want the mesh time
        waveguide_visual_callback = waveguide::postprocessor::visualiser(
                [=](const auto& pressures, size_t step) {
                    //  convert step to time
                    callback(pressures, step / waveguide_sample_rate);
                });
    }

    void unregister_waveguide_visual_callback() {
        waveguide_visual_callback = waveguide::step_postprocessor{};
    }

    void register_raytracer_visual_callback(
            raytracer_visual_callback_t callback) {
        raytracer_visual_callback = callback;
    }

    void unregister_raytracer_visual_callback() {
        raytracer_visual_callback = raytracer_visual_callback_t{};
    }

private:
    compute_context compute_context;

    float mesh_spacing;

    voxelised_scene_data voxelised;
    waveguide::mesh::model model;

    glm::vec3 source;
    glm::vec3 receiver;
    double waveguide_sample_rate;
    size_t rays;
    size_t impulses;

    size_t source_index;
    size_t receiver_index;

    waveguide::step_postprocessor waveguide_visual_callback;
    raytracer_visual_callback_t raytracer_visual_callback;
};

engine::engine(const compute_context& compute_context,
               const copyable_scene_data& scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               double waveguide_sample_rate,
               size_t rays,
               size_t impulses)
        : pimpl(std::make_unique<impl>(compute_context,
                                       scene_data,
                                       source,
                                       receiver,
                                       waveguide_sample_rate,
                                       rays,
                                       impulses)) {}

engine::engine(const compute_context& compute_context,
               const copyable_scene_data& scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               double waveguide_sample_rate,
               size_t rays)
        : pimpl(std::make_unique<impl>(compute_context,
                                       scene_data,
                                       source,
                                       receiver,
                                       waveguide_sample_rate,
                                       rays)) {}

engine::~engine() noexcept = default;

std::unique_ptr<intermediate> engine::run(
        std::atomic_bool& keep_going, const engine::state_callback& callback) {
    return pimpl->run(keep_going, callback);
}

aligned::vector<glm::vec3> engine::get_node_positions() const {
    return pimpl->get_node_positions();
}

void engine::register_raytracer_visual_callback(
        raytracer_visual_callback_t callback) {
    pimpl->register_raytracer_visual_callback(std::move(callback));
}

void engine::unregister_raytracer_visual_callback() {
    pimpl->unregister_raytracer_visual_callback();
}

void engine::register_waveguide_visual_callback(
        waveguide_visual_callback_t callback) {
    pimpl->register_waveguide_visual_callback(std::move(callback));
}

void engine::unregister_waveguide_visual_callback() {
    pimpl->unregister_waveguide_visual_callback();
}

void engine::swap(engine& rhs) noexcept {
    using std::swap;
    swap(pimpl, rhs.pimpl);
}

void swap(engine& a, engine& b) noexcept { a.swap(b); }

}  // namespace wayverb
