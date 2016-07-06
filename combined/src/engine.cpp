#include "combined/engine.h"

#include "common/config.h"
#include "common/kernel.h"

#include <cmath>

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

namespace engine {

template <BufferType buffer_type>
WayverbEngine<buffer_type>::WayverbEngine(ComputeContext& compute_context,
                                          const CopyableSceneData& scene_data,
                                          const glm::vec3& source,
                                          const glm::vec3& mic,
                                          float waveguide_sample_rate,
                                          int rays,
                                          int impulses,
                                          float output_sample_rate)
        : scene_data(scene_data)
        , raytracer(RaytracerProgram(compute_context.context,
                                     compute_context.device))
        , waveguide(RectangularProgram(compute_context.context,
                                       compute_context.device),
                    MeshBoundary(scene_data),
                    mic,
                    waveguide_sample_rate)
        , source(source)
        , mic(mic)
        , waveguide_sample_rate(waveguide_sample_rate)
        , rays(rays)
        , impulses(impulses)
        , source_index(waveguide.get_index_for_coordinate(source))
        , mic_index(waveguide.get_index_for_coordinate(mic)) {
}

template <BufferType buffer_type>
bool WayverbEngine<buffer_type>::get_source_position_is_valid() const {
    return waveguide.inside(source_index);
}

template <BufferType buffer_type>
bool WayverbEngine<buffer_type>::get_mic_position_is_valid() const {
    return waveguide.inside(mic_index);
}

template <BufferType buffer_type>
void WayverbEngine<buffer_type>::check_source_mic_positions() const {
    if (!get_source_position_is_valid()) {
        throw std::runtime_error(
                "invalid source position - probably outside mesh");
    }
    if (!get_mic_position_is_valid()) {
        throw std::runtime_error(
                "invalid mic position - probably outside mesh");
    }
}

template <BufferType buffer_type>
typename WayverbEngine<buffer_type>::Intermediate
WayverbEngine<buffer_type>::run_visualised(
        std::atomic_bool& keep_going,
        const WayverbEngine::StateCallback& callback,
        const WayverbEngine::VisualiserCallback& visualiser_callback) {
    auto waveguide_step = 0;
    return this->run_basic(
            keep_going,
            callback,
            [&waveguide_step, &visualiser_callback](
                    RectangularWaveguide<buffer_type>& waveguide,
                    const glm::vec3& corrected_source,
                    std::vector<float>&& input,
                    size_t mic_index,
                    size_t steps,
                    std::atomic_bool& keep_going,
                    const StateCallback& callback) {
                return waveguide.init_and_run_visualised(
                        corrected_source,
                        std::move(input),
                        mic_index,
                        steps,
                        keep_going,
                        [&callback, &waveguide_step, steps] {
                            callback(State::running_waveguide,
                                     waveguide_step++ / (steps - 1.0));
                        },
                        visualiser_callback);
            });
}

template <BufferType buffer_type>
typename WayverbEngine<buffer_type>::Intermediate
WayverbEngine<buffer_type>::run(std::atomic_bool& keep_going,
                                const WayverbEngine::StateCallback& callback) {
    auto waveguide_step = 0;
    return this->run_basic(
            keep_going,
            callback,
            [&waveguide_step](RectangularWaveguide<buffer_type>& waveguide,
                              const glm::vec3& corrected_source,
                              std::vector<float>&& input,
                              size_t mic_index,
                              size_t steps,
                              std::atomic_bool& keep_going,
                              const StateCallback& callback) {
                return waveguide.init_and_run(
                        corrected_source,
                        std::move(input),
                        mic_index,
                        steps,
                        keep_going,
                        [&callback, &waveguide_step, steps] {
                            callback(State::running_waveguide,
                                     waveguide_step++ / (steps - 1.0));
                        });
            });
}

template <BufferType buffer_type>
template <typename Callback>
auto WayverbEngine<buffer_type>::run_basic(std::atomic_bool& keep_going,
                                           const StateCallback& callback,
                                           const Callback& waveguide_callback) {
    check_source_mic_positions();  //  will throw if the positions are invalid

    //  RAYTRACER  -----------------------------------------------------------//
    callback(State::starting_raytracer, 1.0);
    auto raytracer_step = 0;
    auto raytracer_results =
            raytracer.run(scene_data,
                          mic,
                          source,
                          rays,
                          impulses,
                          keep_going,
                          [this, &callback, &raytracer_step] {
                              callback(State::running_raytracer,
                                       raytracer_step++ / (impulses - 1.0));
                          });
    callback(State::finishing_raytracer, 1.0);

    auto impulses = raytracer_results.get_all(false).impulses;

    //  look for the max time of an impulse
    auto max_time =
            proc::max_element(impulses, [](const auto& a, const auto& b) {
                return a.time < b.time;
            })->time;

    //  this is the number of steps to run the raytracer for
    //  TODO is there a less dumb way of doing this?
    auto steps = std::ceil(max_time * waveguide_sample_rate);

    //  WAVEGUIDE  -----------------------------------------------------------//
    callback(State::starting_waveguide, 1.0);
    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    auto input = kernels::sin_modulated_gaussian_kernel(waveguide_sample_rate);
    auto correction_amount = std::ceil(input.size() / 2.0);

    //  If the max raytracer time is large this could take forever...
    auto waveguide_results = waveguide_callback(waveguide,
                                                corrected_source,
                                                std::move(input),
                                                mic_index,
                                                steps,
                                                keep_going,
                                                callback);
    callback(State::finishing_waveguide, 1.0);

    auto output = std::vector<float>(waveguide_results.size());
    proc::transform(waveguide_results, output.begin(), [](const auto& i) {
        return i.pressure;
    });

    //  correct for filter time offset
    output.erase(output.begin(), output.begin() + correction_amount);

    Intermediate ret;
    return ret;
}

template <BufferType buffer_type>
std::vector<std::vector<float>> WayverbEngine<buffer_type>::attenuate(
        const Intermediate& i,
        //  other args or whatever
        const StateCallback& callback) {
    callback(State::postprocessing, 1.0);

    //  TODO attenuate raytracer results
    //  TODO attenuate waveguide results
    //  TODO scale waveguide output to match raytracer level
    //  TODO crossover filtering
    //  TODO mixdown

    std::vector<std::vector<float>> ret;
    return ret;
}

//  instantiate
template class WayverbEngine<BufferType::cl>;

}  // namespace engine
