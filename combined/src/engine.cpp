#include "combined/engine.h"

#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/config.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/receiver_settings.h"
#include "common/map.h"

#include "raytracer/attenuator.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "waveguide/attenuator/hrtf.h"
#include "waveguide/attenuator/microphone.h"
#include "waveguide/default_kernel.h"
#include "waveguide/postprocess.h"
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
    intermediate_impl(const glm::vec3& source,
                      const glm::vec3& receiver,
                      double waveguide_sample_rate,
                      raytracer::results&& raytracer_results,
                      aligned::vector<waveguide::run_step_output>&&
                              waveguide_results)
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
        LOG(INFO) << "starting attenuation";
        callback(wayverb::state::postprocessing, 1.0);

        //  attenuate raytracer results
        auto raytracer_output = raytracer::run_attenuation(
                cc, receiver, raytracer_results, output_sample_rate);
        //  attenuate waveguide results
        auto waveguide_output = waveguide::run_attenuation(
                receiver, waveguide_results, waveguide_sample_rate);

        //  correct raytracer results for dc
        filter::ExtraLinearDCBlocker blocker;
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

float max_reflectivity(const VolumeType& vt) {
    return *proc::max_element(vt.s);
}

float max_reflectivity(const Surface& surface) {
    return std::max(max_reflectivity(surface.diffuse),
                    max_reflectivity(surface.specular));
}

float max_reflectivity(const SceneData::Material& material) {
    return max_reflectivity(material.surface);
}

float max_reflectivity(const aligned::vector<SceneData::Material>& materials) {
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
         const CopyableSceneData& scene_data,
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
         const CopyableSceneData& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays,
         size_t impulses)
            : compute_context(cc)
            , scene_data(scene_data)
            , raytracer(cc.get_context(), cc.get_device())
            , waveguide(cc.get_context(),
                        cc.get_device(),
                        MeshBoundary(scene_data),
                        receiver,
                        waveguide_sample_rate)
            , source(source)
            , receiver(receiver)
            , waveguide_sample_rate(waveguide_sample_rate)
            , rays(rays)
            , impulses(impulses)
            , source_index(waveguide.get_index_for_coordinate(source))
            , receiver_index(waveguide.get_index_for_coordinate(receiver)) {
        if (!waveguide.inside(source_index)) {
            throw std::runtime_error(
                    "invalid source position - probably outside mesh");
        }
        if (!waveguide.inside(receiver_index)) {
            throw std::runtime_error(
                    "invalid receiver position - probably outside mesh");
        }
    }

    std::unique_ptr<intermediate> run(std::atomic_bool& keep_going,
                                      const state_callback& callback) {
        auto waveguide_step = 0u;
        return this->run_basic(
                keep_going,
                callback,
                [&waveguide_step](waveguide::waveguide& waveguide,
                                  const glm::vec3& corrected_source,
                                  const aligned::vector<float>& input,
                                  size_t mic_index,
                                  size_t steps,
                                  std::atomic_bool& keep_going,
                                  const state_callback& callback) {
                    return waveguide::init_and_run(
                            waveguide,
                            corrected_source,
                            std::move(input),
                            mic_index,
                            steps,
                            keep_going,
                            [&callback, &waveguide_step, steps] {
                                callback(state::running_waveguide,
                                         waveguide_step++ / (steps - 1.0));
                            });
                });
    }

    std::unique_ptr<intermediate> run_visualised(
            std::atomic_bool& keep_going,
            const state_callback& s_callback,
            const visualiser_callback& v_callback) {
        auto waveguide_step = 0u;
        return this->run_basic(
                keep_going,
                s_callback,
                [&waveguide_step, &v_callback](
                        waveguide::waveguide& waveguide,
                        const glm::vec3& corrected_source,
                        const aligned::vector<float>& input,
                        size_t mic_index,
                        size_t steps,
                        std::atomic_bool& keep_going,
                        const state_callback& callback) {
                    return waveguide::init_and_run(
                            waveguide,
                            corrected_source,
                            std::move(input),
                            mic_index,
                            steps,
                            keep_going,
                            [&callback, &waveguide_step, steps] {
                                callback(state::running_waveguide,
                                         waveguide_step++ / (steps - 1.0));
                            },
                            v_callback);
                });
    }

    aligned::vector<glm::vec3> get_node_positions() const {
        return map_to_vector(waveguide.get_mesh().get_nodes(),
                             [](const auto& i) { return to_vec3(i.position); });
    }

private:
    using waveguide_callback = std::function<std::experimental::optional<
            aligned::vector<waveguide::run_step_output>>(
            waveguide::waveguide&,
            const glm::vec3&,
            const aligned::vector<float>&,
            size_t,
            size_t,
            std::atomic_bool&,
            const state_callback&)>;

    std::unique_ptr<intermediate> run_basic(
            std::atomic_bool& keep_going,
            const state_callback& callback,
            const waveguide_callback& waveguide_callback) {
        //  RAYTRACER  -------------------------------------------------------//
        callback(state::starting_raytracer, 1.0);
        auto raytracer_step    = 0;
        auto raytracer_results = raytracer.run(
                scene_data,
                source,
                receiver,
                rays,
                impulses,
                std::min(size_t{10},
                         impulses),  //  TODO set this more intelligently
                keep_going,
                [this, &callback, &raytracer_step] {
                    callback(state::running_raytracer,
                             raytracer_step++ / (impulses - 1.0));
                });

        if (!(keep_going && raytracer_results)) {
            return nullptr;
        }

        callback(state::finishing_raytracer, 1.0);

        auto impulses = raytracer_results->get_impulses();

        //  look for the max time of an impulse
        auto max_time =
                proc::max_element(impulses, [](const auto& a, const auto& b) {
                    return a.time < b.time;
                })->time;

        //  this is the number of steps to run the raytracer for
        //  TODO is there a less dumb way of doing this?
        auto steps = std::ceil(max_time * waveguide_sample_rate);

        //  WAVEGUIDE  -------------------------------------------------------//
        callback(state::starting_waveguide, 1.0);

        const auto corrected_source =
                waveguide.get_coordinate_for_index(source_index);
        const auto input = waveguide::default_kernel(waveguide_sample_rate);

        //  If the max raytracer time is large this could take forever...
        auto waveguide_results =
                waveguide_callback(waveguide,
                                   corrected_source,
                                   input.kernel,
                                   receiver_index,
                                   steps + input.opaque_kernel_size,
                                   keep_going,
                                   callback);

        if (!(keep_going && waveguide_results)) {
            return nullptr;
        }

        callback(state::finishing_waveguide, 1.0);

        //  correct for filter time offset
        assert(input.correction_offset_in_samples < waveguide_results->size());

        LOG(INFO) << "correcting waveguide for kernel time offset";
        waveguide_results->erase(waveguide_results->begin(),
                                 waveguide_results->begin() +
                                         input.correction_offset_in_samples);

        LOG(INFO) << "creating ret";

        std::unique_ptr<intermediate> ret = std::make_unique<intermediate_impl>(
                source,
                receiver,
                waveguide_sample_rate,
                std::move(*raytracer_results),
                std::move(*waveguide_results));

        LOG(INFO) << "about to return from run_basic";

        return ret;
    }

    compute_context compute_context;
    CopyableSceneData scene_data;
    raytracer::raytracer raytracer;
    waveguide::waveguide waveguide;

    glm::vec3 source;
    glm::vec3 receiver;
    double waveguide_sample_rate;
    size_t rays;
    size_t impulses;

    size_t source_index;
    size_t receiver_index;
};

engine::engine(const compute_context& compute_context,
               const CopyableSceneData& scene_data,
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
               const CopyableSceneData& scene_data,
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

std::unique_ptr<intermediate> engine::run_visualised(
        std::atomic_bool& keep_going,
        const engine::state_callback& callback,
        const engine::visualiser_callback& visualiser_callback) {
    return pimpl->run_visualised(keep_going, callback, visualiser_callback);
}

std::unique_ptr<intermediate> engine::run(
        std::atomic_bool& keep_going, const engine::state_callback& callback) {
    return pimpl->run(keep_going, callback);
}

aligned::vector<glm::vec3> engine::get_node_positions() const {
    return pimpl->get_node_positions();
}

void engine::swap(engine& rhs) noexcept {
    using std::swap;
    swap(pimpl, rhs.pimpl);
}

void swap(engine& a, engine& b) noexcept { a.swap(b); }

}  // namespace wayverb
