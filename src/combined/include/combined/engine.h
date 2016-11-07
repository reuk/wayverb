#pragma once

#include "combined/postprocess.h"

#include "raytracer/canonical.h"

#include "combined/polymorphic_waveguide.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/attenuator/null.h"
#include "core/cl/common.h"
#include "core/environment.h"
#include "core/scene_data.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"

#include "glm/glm.hpp"

#include <functional>

namespace wayverb {
namespace combined {

//  state information  /////////////////////////////////////////////////////////

enum class state {
    idle,
    initialising,
    starting_raytracer,
    running_raytracer,
    finishing_raytracer,
    starting_waveguide,
    running_waveguide,
    finishing_waveguide,
    postprocessing,
};

constexpr auto to_string(state s) {
    switch (s) {
        case state::idle: return "idle";
        case state::initialising: return "initialising";
        case state::starting_raytracer: return "starting raytracer";
        case state::running_raytracer: return "running raytracer";
        case state::finishing_raytracer: return "finishing raytracer";
        case state::starting_waveguide: return "starting waveguide";
        case state::running_waveguide: return "running waveguide";
        case state::finishing_waveguide: return "finishing waveguide";
        case state::postprocessing: return "postprocessing";
    }
}

using engine_state_changed = util::event<state, double>;
using waveguide_node_pressures_changed =
        util::event<util::aligned::vector<float>, double>;
using raytracer_reflections_generated = util::event<
        util::aligned::vector<util::aligned::vector<raytracer::reflection>>>;

//  postprocessing  ////////////////////////////////////////////////////////////

class intermediate {
public:
    intermediate() = default;
    intermediate(const intermediate&) = default;
    intermediate(intermediate&&) noexcept = default;
    intermediate& operator=(const intermediate&) = default;
    intermediate& operator=(intermediate&&) noexcept = default;
    virtual ~intermediate() noexcept = default;

    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::null&, double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf&, double) const = 0;
    virtual util::aligned::vector<float> postprocess(
            const core::attenuator::microphone&, double) const = 0;
};

template <typename Histogram>
class intermediate_impl final : public intermediate {
public:
    intermediate_impl(combined_results<Histogram> to_process,
                      const glm::vec3& receiver_position,
                      double room_volume,
                      const core::environment& environment)
            : to_process_{std::move(to_process)}
            , receiver_position_{receiver_position}
            , room_volume_{room_volume}
            , environment_{environment} {}

    util::aligned::vector<float> postprocess(
            const core::attenuator::null& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::hrtf& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
    }

    util::aligned::vector<float> postprocess(
            const core::attenuator::microphone& a,
            double sample_rate) const override {
        return postprocess_impl(a, sample_rate);
    }

private:
    template <typename Attenuator>
    auto postprocess_impl(const Attenuator& attenuator,
                          double output_sample_rate) const {
        return wayverb::combined::postprocess(to_process_,
                                              attenuator,
                                              receiver_position_,
                                              room_volume_,
                                              environment_,
                                              output_sample_rate);
    }

    combined_results<Histogram> to_process_;
    glm::vec3 receiver_position_;
    double room_volume_;
    core::environment environment_;
    engine_state_changed engine_state_changed_;
};

template <typename Histogram>
auto make_intermediate_impl_ptr(combined_results<Histogram> to_process,
                                const glm::vec3& receiver_position,
                                double room_volume,
                                const core::environment& environment) {
    return std::make_unique<intermediate_impl<Histogram>>(
            std::move(to_process), receiver_position, room_volume, environment);
}

//  engine  ////////////////////////////////////////////////////////////////////

class engine final {
public:
    engine(const core::compute_context& compute_context,
           const core::gpu_scene_data& scene_data,
           const glm::vec3& source,
           const glm::vec3& receiver,
           const core::environment& environment,
           const raytracer::simulation_parameters& raytracer,
           std::unique_ptr<waveguide_base> waveguide);

    //  Only valid when WaveguideParameters is
    //  waveguide::single_band_parameters
    //  waveguide::multiple_band_constant_spacing_parameters

    std::unique_ptr<intermediate> run(const std::atomic_bool& keep_going) const;

    //  notifications  /////////////////////////////////////////////////////////

    engine_state_changed::connection add_engine_state_changed_callback(
            engine_state_changed::callback_type callback);

    waveguide_node_pressures_changed::connection
    add_waveguide_node_pressures_changed_callback(
            waveguide_node_pressures_changed::callback_type callback);

    raytracer_reflections_generated::connection
    add_raytracer_reflections_generated_callback(
            raytracer_reflections_generated::callback_type callback);

    //  cached data  ///////////////////////////////////////////////////////////

    const waveguide::voxels_and_mesh& get_voxels_and_mesh() const;

private:
    core::compute_context compute_context_;
    waveguide::voxels_and_mesh voxels_and_mesh_;
    double room_volume_;
    glm::vec3 source_;
    glm::vec3 receiver_;
    core::environment environment_;
    raytracer::simulation_parameters raytracer_;
    std::unique_ptr<waveguide_base> waveguide_;

    engine_state_changed engine_state_changed_;
    waveguide_node_pressures_changed waveguide_node_pressures_changed_;
    raytracer_reflections_generated raytracer_reflections_generated_;
};

}  // namespace combined
}  // namespace wayverb
