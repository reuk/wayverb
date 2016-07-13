#include "combined/engine.h"

#include "raytracer/raytracer.h"

#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/config.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/kernel.h"
#include "common/receiver_settings.h"

#include "waveguide/default_kernel.h"
#include "waveguide/hrtf_attenuator.h"
#include "waveguide/microphone_attenuator.h"
#include "waveguide/rectangular_waveguide.h"

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

auto run_raytracer_attenuation(const RaytracerProgram& program,
                               const model::ReceiverSettings& receiver,
                               const raytracer::RaytracerResults& input) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            raytracer::MicrophoneAttenuator attenuator(program);
            return run_attenuation(
                    receiver.microphones.begin(),
                    receiver.microphones.end(),
                    [&receiver, &input, &attenuator](const auto& i) {
                        return attenuator.process(
                                input,
                                i.pointer.get_pointing(receiver.position),
                                i.shape,
                                receiver.position);
                    });
        }
        case model::ReceiverSettings::Mode::hrtf: {
            raytracer::HrtfAttenuator attenuator(program);
            auto channels = {HrtfChannel::left, HrtfChannel::right};
            return run_attenuation(
                    channels.begin(),
                    channels.end(),
                    [&receiver, &input, &attenuator](const auto& i) {
                        return attenuator.process(
                                input,
                                receiver.hrtf.get_pointing(receiver.position),
                                glm::vec3(0, 1, 0),
                                receiver.position,
                                i);
                    });
        }
    }
}

auto run_waveguide_attenuation(const model::ReceiverSettings& receiver,
                               const std::vector<RunStepResult>& input,
                               double waveguide_sample_rate) {
    switch (receiver.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            waveguide::MicrophoneAttenuator attenuator;
            return run_attenuation(
                    receiver.microphones.begin(),
                    receiver.microphones.end(),
                    [&receiver, &input, &attenuator](const auto& i) {
                        return attenuator.process(
                                input,
                                i.pointer.get_pointing(receiver.position),
                                i.shape);
                    });
        }
        case model::ReceiverSettings::Mode::hrtf: {
            waveguide::HrtfAttenuator attenuator;
            auto channels = {HrtfChannel::left, HrtfChannel::right};
            return run_attenuation(
                    channels.begin(),
                    channels.end(),
                    [&receiver, &input, &attenuator, waveguide_sample_rate](
                            const auto& i) {
                        auto ret = attenuator.process(
                                input,
                                receiver.hrtf.get_pointing(receiver.position),
                                glm::vec3(0, 1, 0),
                                i);
                        return multiband_filter_and_mixdown(
                                ret, waveguide_sample_rate);
                    });
        }
    }
}
}  // namespace

namespace wayverb {

template <BufferType buffer_type>
class engine<buffer_type>::impl final {
public:
    impl(const ComputeContext& compute_context,
         const CopyableSceneData& scene_data,
         const glm::vec3& source,
         const glm::vec3& receiver,
         float waveguide_sample_rate,
         int rays,
         int impulses,
         float output_sample_rate)
            : scene_data(scene_data)
            , raytracer_program(compute_context.context, compute_context.device)
            , raytracer(raytracer_program)
            , waveguide(RectangularProgram(compute_context.context,
                                           compute_context.device),
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
    }

    bool get_source_position_is_valid() const {
        return waveguide.inside(source_index);
    }
    bool get_receiver_position_is_valid() const {
        return waveguide.inside(receiver_index);
    }

    std::unique_ptr<intermediate> run(std::atomic_bool& keep_going,
                                      const state_callback& callback) {
        auto waveguide_step = 0;
        return this->run_basic(
                keep_going,
                callback,
                [&waveguide_step](RectangularWaveguide<buffer_type>& waveguide,
                                  const glm::vec3& corrected_source,
                                  const std::vector<float>& input,
                                  size_t mic_index,
                                  size_t steps,
                                  std::atomic_bool& keep_going,
                                  const state_callback& callback) {
                    return waveguide.init_and_run(
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
        auto waveguide_step = 0;
        return this->run_basic(
                keep_going,
                s_callback,
                [&waveguide_step, &v_callback](
                        RectangularWaveguide<buffer_type>& waveguide,
                        const glm::vec3& corrected_source,
                        const std::vector<float>& input,
                        size_t mic_index,
                        size_t steps,
                        std::atomic_bool& keep_going,
                        const state_callback& callback) {
                    return waveguide.init_and_run_visualised(
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

    std::vector<std::vector<float>> attenuate(
            const intermediate& i,
            const model::ReceiverSettings& receiver,
            const state_callback& callback) {
        callback(state::postprocessing, 1.0);

        //  attenuate raytracer results
        auto raytracer_output = run_raytracer_attenuation(
                raytracer_program, receiver, i.raytracer_results);
        //  attenuate waveguide results
        auto waveguide_output = run_waveguide_attenuation(
                receiver, i.waveguide_results, i.waveguide_sample_rate);

        //  correct waveguide results for dc
        filter::ExtraLinearDCBlocker blocker;
        for (auto& i : waveguide_output) {
            i = filter::run_two_pass(blocker, i.begin(), i.end());
        }
        //  TODO scale waveguide output to match raytracer level
        //  TODO crossover filtering
        //  TODO mixdown

        std::vector<std::vector<float>> ret;
        return ret;
    }

    std::vector<cl_float3> get_node_positions() const {
        const auto& nodes = waveguide.get_mesh().get_nodes();
        std::vector<cl_float3> ret(nodes.size());
        proc::transform(
                nodes, ret.begin(), [](const auto& i) { return i.position; });
        return ret;
    }

private:
    void check_source_receiver_positions() const {
        if (!get_source_position_is_valid()) {
            throw std::runtime_error(
                    "invalid source position - probably outside mesh");
        }
        if (!get_receiver_position_is_valid()) {
            throw std::runtime_error(
                    "invalid receiver position - probably outside mesh");
        }
    }

    auto run_basic(std::atomic_bool& keep_going,
                   const state_callback& callback,
                   const std::function<std::vector<RunStepResult>(
                           RectangularWaveguide<buffer_type>&,
                           const glm::vec3&,
                           const std::vector<float>&,
                           size_t,
                           size_t,
                           std::atomic_bool&,
                           const state_callback&)>& waveguide_callback) {
        check_source_receiver_positions();  //  will throw if the positions are
                                            //  invalid

        //  RAYTRACER  -------------------------------------------------------//
        callback(state::starting_raytracer, 1.0);
        auto raytracer_step = 0;
        auto raytracer_results =
                raytracer.run(scene_data,
                              receiver,
                              source,
                              rays,
                              impulses,
                              keep_going,
                              [this, &callback, &raytracer_step] {
                                  callback(state::running_raytracer,
                                           raytracer_step++ / (impulses - 1.0));
                              });
        callback(state::finishing_raytracer, 1.0);

        auto impulses = raytracer_results.get_all(false).impulses;

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
        auto corrected_source =
                waveguide.get_coordinate_for_index(source_index);

        auto input = default_kernel(waveguide_sample_rate);

        //  If the max raytracer time is large this could take forever...
        auto waveguide_results =
                waveguide_callback(waveguide,
                                   corrected_source,
                                   input.kernel,
                                   receiver_index,
                                   steps + input.opaque_kernel_size,
                                   keep_going,
                                   callback);
        callback(state::finishing_waveguide, 1.0);

        //  correct for filter time offset
        waveguide_results.erase(
                waveguide_results.begin(),
                waveguide_results.begin() + input.correction_offset_in_samples);

        return std::make_unique<intermediate>();
    }

    CopyableSceneData scene_data;
    RaytracerProgram raytracer_program;
    raytracer::Raytracer raytracer;
    RectangularWaveguide<buffer_type> waveguide;

    glm::vec3 source;
    glm::vec3 receiver;
    float waveguide_sample_rate;
    int rays;
    int impulses;
    // float output_sample_rate;

    size_t source_index;
    size_t receiver_index;
};

template <BufferType buffer_type>
engine<buffer_type>::engine(const ComputeContext& compute_context,
                            const CopyableSceneData& scene_data,
                            const glm::vec3& source,
                            const glm::vec3& receiver,
                            float waveguide_sample_rate,
                            int rays,
                            int impulses,
                            float output_sample_rate)
        : pimpl(std::make_unique<impl>(compute_context,
                                       scene_data,
                                       source,
                                       receiver,
                                       waveguide_sample_rate,
                                       rays,
                                       impulses,
                                       output_sample_rate)) {
}

template <BufferType buffer_type>
engine<buffer_type>::~engine() noexcept = default;

template <BufferType buffer_type>
bool engine<buffer_type>::get_source_position_is_valid() const {
    return pimpl->get_source_position_is_valid();
}

template <BufferType buffer_type>
bool engine<buffer_type>::get_receiver_position_is_valid() const {
    return pimpl->get_receiver_position_is_valid();
}

template <BufferType buffer_type>
std::unique_ptr<typename engine<buffer_type>::intermediate>
engine<buffer_type>::run_visualised(
        std::atomic_bool& keep_going,
        const engine::state_callback& callback,
        const engine::visualiser_callback& visualiser_callback) {
    return pimpl->run_visualised(keep_going, callback, visualiser_callback);
}

template <BufferType buffer_type>
std::unique_ptr<typename engine<buffer_type>::intermediate>
engine<buffer_type>::run(std::atomic_bool& keep_going,
                         const engine::state_callback& callback) {
    return pimpl->run(keep_going, callback);
}

template <BufferType buffer_type>
std::vector<std::vector<float>> engine<buffer_type>::attenuate(
        const intermediate& i,
        const model::ReceiverSettings& receiver,
        const state_callback& callback) {
    return pimpl->attenuate(i, receiver, callback);
}

template <BufferType buffer_type>
std::vector<cl_float3> engine<buffer_type>::get_node_positions() const {
    return pimpl->get_node_positions();
}

//  instantiate
template class engine<BufferType::cl>;

}  // namespace wayverb
