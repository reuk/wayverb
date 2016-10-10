#include "combined/engine.h"

#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/postprocessor/visualiser.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/setup.h"
#include "waveguide/waveguide.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/model/receiver_settings.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/surfaces.h"

#include "hrtf/multiband.h"

#include <cmath>

namespace {

class intermediate_impl : public wayverb::intermediate {
public:
    static constexpr auto bands{8};

    intermediate_impl(
            const glm::vec3& source,
            const glm::vec3& receiver,
            double speed_of_sound,
            double acoustic_impedance,
            double waveguide_sample_rate,
            raytracer::results<impulse<bands>> raytracer_results,
            aligned::vector<
                    waveguide::postprocessor::directional_receiver::output>
                    waveguide_results)
            : source_{source}
            , receiver_{receiver}
            , speed_of_sound_{speed_of_sound}
            , acoustic_impedance_{acoustic_impedance}
            , waveguide_sample_rate_{waveguide_sample_rate}
            , raytracer_results_{std::move(raytracer_results)}
            , waveguide_results_{std::move(waveguide_results)} {}

    aligned::vector<aligned::vector<float>> attenuate(
            const compute_context& cc,
            const model::receiver_settings& receiver,
            double output_sample_rate,
            double max_length_in_seconds,
            const state_callback& callback) const override {
        callback(wayverb::state::postprocessing, 1.0);

        //  attenuate raytracer results
        const auto raytracer_impulses{raytracer_results_.get_impulses()};
        auto raytracer_output{
                raytracer::run_attenuation(receiver,
                                           speed_of_sound_,
                                           output_sample_rate,
                                           max_length_in_seconds,
                                           raytracer_impulses.begin(),
                                           raytracer_impulses.end())};
        //  attenuate waveguide results
        auto waveguide_output{
                waveguide::run_attenuation(receiver,
                                           acoustic_impedance_,
                                           waveguide_sample_rate_,
                                           waveguide_results_.begin(),
                                           waveguide_results_.end())};

        //  correct raytracer results for dc
        filter::extra_linear_dc_blocker blocker;
        for (auto& i : raytracer_output) {
            filter::run_two_pass(blocker, i.begin(), i.end());
        }

        //  correct waveguide results for dc
        for (auto& i : waveguide_output) {
            filter::run_two_pass(blocker, i.begin(), i.end());
        }

        //  correct waveguide sampling rate
        for (auto& i : waveguide_output) {
            i = waveguide::adjust_sampling_rate(i.data(),
                                                i.data() + i.size(),
                                                waveguide_sample_rate_,
                                                output_sample_rate);
        }

        //  convert waveguide output from intensity level back to pressure lvl
        for (auto& i : waveguide_output) {
            for (auto& j : i) {
                j = intensity_to_pressure(
                        j, static_cast<float>(acoustic_impedance_));
            }
        }

        //  TODO scale waveguide output to match raytracer level
        //  TODO crossover filtering

        //  mixdown
        assert(raytracer_output.size() == waveguide_output.size());
        auto mm{std::minmax(raytracer_output,
                            waveguide_output,
                            [](const auto& a, const auto& b) {
                                return a.front().size() < b.front().size();
                            })};

        auto& min{mm.first};
        auto& max{mm.second};

        aligned::vector<aligned::vector<float>> ret;
        ret.reserve(max.size());
        std::transform(
                begin(max),
                end(max),
                min.begin(),
                std::back_inserter(ret),
                [](const auto& a, const auto& b) {
                    auto ret{a};
                    std::transform(begin(b),
                                   end(b),
                                   a.begin(),
                                   ret.begin(),
                                   [](auto a, auto b) { return a + b; });
                    return ret;
                });

        return max;
    }

private:
    glm::vec3 source_;
    glm::vec3 receiver_;
    double speed_of_sound_;
    double acoustic_impedance_;
    double waveguide_sample_rate_;

    raytracer::results<impulse<8>> raytracer_results_;
    aligned::vector<waveguide::postprocessor::directional_receiver::output>
            waveguide_results_;
};

}  // namespace

namespace wayverb {

class engine::impl final {
public:
    impl(const compute_context& cc,
         const scene_data& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays,
         double speed_of_sound,
         double acoustic_impedance)
            : impl(cc,
                   scene_data,
                   source,
                   receiver,
                   waveguide_sample_rate,
                   rays,
                   raytracer::compute_optimum_reflection_number(scene_data),
                   speed_of_sound,
                   acoustic_impedance) {}

    impl(const compute_context& cc,
         const scene_data& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays,
         size_t impulses,
         double speed_of_sound,
         double acoustic_impedance)
            : impl(cc,
                   waveguide::compute_voxels_and_mesh(cc,
                                                      scene_data,
                                                      receiver,
                                                      waveguide_sample_rate,
                                                      speed_of_sound),
                   source,
                   receiver,
                   waveguide_sample_rate,
                   rays,
                   impulses,
                   speed_of_sound,
                   acoustic_impedance) {}

private:
    impl(const compute_context& cc,
         std::tuple<voxelised_scene_data<cl_float3, surface>, waveguide::mesh>&&
                 pair,
         const glm::vec3& source,
         const glm::vec3& receiver,
         double waveguide_sample_rate,
         size_t rays,
         size_t impulses,
         double speed_of_sound,
         double acoustic_impedance)
            : compute_context_{cc}
            , voxelised_{std::move(std::get<0>(pair))}
            , mesh_{std::move(std::get<1>(pair))}
            , speed_of_sound_{speed_of_sound}
            , acoustic_impedance_{acoustic_impedance}
            , source_{source}
            , receiver_{receiver}
            , waveguide_sample_rate_{waveguide_sample_rate}
            , rays_{rays}
            , impulses_{impulses}
            , source_index_{compute_index(mesh_.get_descriptor(), source)}
            , receiver_index_{compute_index(mesh_.get_descriptor(), receiver)} {
        const auto is_index_inside{[&](auto index) {
            return waveguide::is_inside(
                    mesh_.get_structure().get_condensed_nodes()[index]);
        }};

        if (!is_index_inside(source_index_)) {
            throw std::runtime_error(
                    "invalid source position - probably outside mesh");
        }
        if (!is_index_inside(receiver_index_)) {
            throw std::runtime_error(
                    "invalid receiver position - probably outside mesh");
        }
    }

public:
    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going,
                                      const state_callback& callback) const {
        //  RAYTRACER  -------------------------------------------------------//
        callback(state::starting_raytracer, 1.0);
        auto raytracer_results{raytracer::run(
                compute_context_,
                voxelised_,
                source_,
                receiver_,
                get_random_directions(rays_),
                impulses_,
                std::min(size_t{20},
                         impulses_),  //  TODO set this more intelligently
                keep_going,
                [&](auto step) {
                    callback(state::running_raytracer,
                             step / (impulses_ - 1.0));
                })};

        if (!(keep_going && raytracer_results)) {
            return nullptr;
        }

        callback(state::finishing_raytracer, 1.0);

        if (raytracer_visual_callback_) {
            raytracer_visual_callback_(
                    raytracer_results->get_diffuse(), source_, receiver_);
        }

        const auto impulses{raytracer_results->get_impulses()};

        //  look for the max time of an impulse
        const auto max_time{std::max_element(begin(impulses),
                                             end(impulses),
                                             [](const auto& a, const auto& b) {
                                                 return a.distance < b.distance;
                                             })
                                    ->distance /
                            speed_of_sound_};

        //  WAVEGUIDE  -------------------------------------------------------//
        callback(state::starting_waveguide, 1.0);

        const aligned::vector<float> raw_input{1.0f};
        auto input{waveguide::make_transparent(
                raw_input.data(), raw_input.data() + raw_input.size())};

        //  this is the number of steps to run the waveguide for
        //  TODO is there a less dumb way of doing this?
        const auto steps{std::ceil(max_time * waveguide_sample_rate_)};
        input.resize(steps);

        auto prep{waveguide::preprocessor::make_hard_source(
                source_index_, input.begin(), input.end())};

        //  If the max raytracer time is large this could take forever...

        callback_accumulator<waveguide::postprocessor::directional_receiver>
                mic_output{mesh_.get_descriptor(),
                           waveguide_sample_rate_,
                           acoustic_impedance_ / speed_of_sound_,
                           receiver_index_};

        const auto waveguide_steps_completed{waveguide::run(
                compute_context_,
                mesh_,
                prep,
                [&](auto& queue, const auto& buffer, auto step) {
                    mic_output(queue, buffer, step);
                    if (waveguide_visual_callback_) {
                        waveguide_visual_callback_(queue, buffer, step);
                    }
                    callback(state::running_waveguide, step / (steps - 1.0));
                },
                keep_going)};

        if (waveguide_steps_completed != steps) {
            return nullptr;
        }

        //  if we got here, we can assume waveguide_results is complete and
        //  valid

        callback(state::finishing_waveguide, 1.0);

        std::unique_ptr<intermediate> ret{std::make_unique<intermediate_impl>(
                source_,
                receiver_,
                speed_of_sound_,
                acoustic_impedance_,
                waveguide_sample_rate_,
                std::move(*raytracer_results),
                mic_output.get_output())};
        return ret;
    }

    aligned::vector<glm::vec3> get_node_positions() const {
        return compute_node_positions(mesh_.get_descriptor());
    }

    void register_waveguide_visual_callback(
            waveguide_visual_callback_t callback) {
        //  visualiser returns current waveguide step, but we want the mesh time
        waveguide_visual_callback_ = waveguide::postprocessor::visualiser{
                [=](const auto& pressures, size_t step) {
                    //  convert step to time
                    callback(pressures, step / waveguide_sample_rate_);
                }};
    }

    void unregister_waveguide_visual_callback() {
        waveguide_visual_callback_ = waveguide::step_postprocessor{};
    }

    void register_raytracer_visual_callback(
            raytracer_visual_callback_t callback) {
        raytracer_visual_callback_ = callback;
    }

    void unregister_raytracer_visual_callback() {
        raytracer_visual_callback_ = raytracer_visual_callback_t{};
    }

private:
    compute_context compute_context_;

    voxelised_scene_data<cl_float3, surface> voxelised_;
    waveguide::mesh mesh_;

    double speed_of_sound_;
    double acoustic_impedance_;

    glm::vec3 source_;
    glm::vec3 receiver_;
    double waveguide_sample_rate_;
    size_t rays_;
    size_t impulses_;

    size_t source_index_;
    size_t receiver_index_;

    waveguide::step_postprocessor waveguide_visual_callback_;
    raytracer_visual_callback_t raytracer_visual_callback_;
};

constexpr auto speed_of_sound{340};
constexpr auto acoustic_impedance{400};

engine::engine(const compute_context& compute_context,
               const scene_data& scene_data,
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
                                       impulses,
                                       speed_of_sound,
                                       acoustic_impedance)) {}

engine::engine(const compute_context& compute_context,
               const scene_data& scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               double waveguide_sample_rate,
               size_t rays)
        : pimpl(std::make_unique<impl>(compute_context,
                                       scene_data,
                                       source,
                                       receiver,
                                       waveguide_sample_rate,
                                       rays,
                                       speed_of_sound,
                                       acoustic_impedance)) {}

engine::~engine() noexcept = default;

std::unique_ptr<intermediate> engine::run(
        const std::atomic_bool& keep_going,
        const engine::state_callback& callback) const {
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
