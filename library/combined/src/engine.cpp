#include "combined/engine.h"
#include "combined/postprocess.h"

#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "waveguide/canonical.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/azimuth_elevation.h"
#include "common/callback_accumulator.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/model/receiver.h"
#include "common/pressure_intensity.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/surfaces.h"

#include "hrtf/multiband.h"

#include <cmath>

namespace {

template <typename Histogram>
class intermediate_impl : public wayverb::intermediate {
public:
    intermediate_impl(wayverb::combined_results<Histogram> to_process,
                      const glm::vec3& receiver_position,
                      double room_volume,
                      double acoustic_impedance,
                      double speed_of_sound)
            : to_process_{std::move(to_process)}
            , receiver_position_{receiver_position}
            , room_volume_{room_volume}
            , acoustic_impedance_{acoustic_impedance}
            , speed_of_sound_{speed_of_sound} {}

    aligned::vector<aligned::vector<float>> attenuate(
            const model::receiver& receiver,
            double output_sample_rate) const override {
        return model::run_attenuation(receiver,
                                      to_process_,
                                      receiver_position_,
                                      room_volume_,
                                      acoustic_impedance_,
                                      speed_of_sound_,
                                      output_sample_rate);
    }

private:
    wayverb::combined_results<Histogram> to_process_;
    glm::vec3 receiver_position_;
    double room_volume_;
    double acoustic_impedance_;
    double speed_of_sound_;
};

template <typename Histogram>
auto make_intermediate_impl_ptr(wayverb::combined_results<Histogram> to_process,
                                const glm::vec3& receiver_position,
                                double room_volume,
                                double acoustic_impedance,
                                double speed_of_sound) {
    return std::make_unique<intermediate_impl<Histogram>>(std::move(to_process),
                                                          receiver_position,
                                                          room_volume,
                                                          acoustic_impedance,
                                                          speed_of_sound);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

namespace wayverb {

class engine::impl final {
public:
    impl(const compute_context& cc,
         const scene_data& scene_data,
         const model::parameters& params,
         double waveguide_sample_rate,
         size_t rays)
            : impl(cc,
                   waveguide::compute_voxels_and_mesh(cc,
                                                      scene_data,
                                                      params.receiver,
                                                      waveguide_sample_rate,
                                                      params.speed_of_sound),
                   params,
                   waveguide_sample_rate,
                   rays) {}

private:
    impl(const compute_context& cc,
         waveguide::voxels_and_mesh pair,
         const model::parameters& params,
         double waveguide_sample_rate,
         size_t rays)
            : compute_context_{cc}
            , voxelised_{std::move(pair.voxels)}
            , mesh_{std::move(pair.mesh)}
            , params_{params}
            , room_volume_{estimate_room_volume(pair.voxels.get_scene_data())}
            , waveguide_sample_rate_{waveguide_sample_rate}
            , rays_{rays} {}

public:
    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going,
                                      const state_callback& callback) const {
        //  RAYTRACER  /////////////////////////////////////////////////////////

        const auto max_image_source_order = 5;
        const auto rays_to_visualise = std::min(1000ul, rays_);
        const auto directions = get_random_directions(rays_);

        callback(state::starting_raytracer, 1.0);

        auto raytracer_output =
                raytracer::canonical(begin(directions),
                                     end(directions),
                                     compute_context_,
                                     voxelised_,
                                     params_,
                                     max_image_source_order,
                                     rays_to_visualise,
                                     keep_going,
                                     [&](auto step, auto total_steps) {
                                         callback(state::running_raytracer,
                                                  step / (total_steps - 1.0));
                                     });

        if (!(keep_going && raytracer_output)) {
            return nullptr;
        }

        callback(state::finishing_raytracer, 1.0);

        if (raytracer_visual_callback_) {
            raytracer_visual_callback_(std::move(raytracer_output->visual),
                                       params_.source,
                                       params_.receiver);
        }

        //  look for the max time of an impulse
        const auto max_stochastic_time =
                max_time(raytracer_output->aural.stochastic);

        //  WAVEGUIDE  /////////////////////////////////////////////////////////
        callback(state::starting_waveguide, 1.0);

        auto waveguide_output = waveguide::detail::canonical_impl(
                compute_context_,
                mesh_,
                waveguide_sample_rate_,
                max_stochastic_time,
                params_,
                keep_going,
                [&](auto step, auto steps) {
                    callback(state::running_raytracer, step / (steps - 1.0));
                });

        if (!(keep_going && waveguide_output)) {
            return nullptr;
        }

        callback(state::finishing_waveguide, 1.0);

        return make_intermediate_impl_ptr(
                make_combined_results(
                        std::move(raytracer_output->aural),
                        waveguide_results{std::move(*waveguide_output),
                                          waveguide_sample_rate_}),
                params_.receiver,
                room_volume_,
                params_.acoustic_impedance,
                params_.speed_of_sound);
    }

    aligned::vector<glm::vec3> get_node_positions() const {
        return compute_node_positions(mesh_.get_descriptor());
    }

    void register_waveguide_visual_callback(
            waveguide_visual_callback_t callback) {
        //  visualiser returns current waveguide step, but we want the mesh
        //  time
        waveguide_visual_callback_ = [=](
                auto& queue, const auto& buffer, auto step) {
            //  convert step to time
            callback(read_from_buffer<cl_float>(queue, buffer),
                     step / waveguide_sample_rate_);
        };
    }

    void unregister_waveguide_visual_callback() {
        waveguide_visual_callback_ = waveguide_callback_wrapper_t{};
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

    voxelised_scene_data<cl_float3, surface<simulation_bands>> voxelised_;
    waveguide::mesh mesh_;

    model::parameters params_;

    double room_volume_;
    double waveguide_sample_rate_;
    size_t rays_;

    using waveguide_callback_wrapper_t = std::function<void(
            cl::CommandQueue& queue, const cl::Buffer& buffer, size_t step)>;

    waveguide_callback_wrapper_t waveguide_visual_callback_;
    raytracer_visual_callback_t raytracer_visual_callback_;
};

////////////////////////////////////////////////////////////////////////////////

engine::engine(const compute_context& compute_context,
               const scene_data& scene_data,
               const glm::vec3& source,
               const glm::vec3& receiver,
               double waveguide_sample_rate,
               size_t rays)
        : pimpl(std::make_unique<impl>(
                  compute_context,
                  scene_data,
                  model::parameters{source, receiver, 340.0, 400.0},
                  waveguide_sample_rate,
                  rays)) {}

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
