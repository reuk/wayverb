#include "combined/engine.h"
#include "combined/config.h"

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

template <BufferType buffer_type>
WayverbEngine<buffer_type>::WayverbEngine(ComputeContext& compute_context,
                                          const SceneData& scene_data,
                                          const Vec3f& source,
                                          const Vec3f& mic,
                                          float waveguide_sample_rate,
                                          int rays,
                                          int impulses,
                                          float output_sample_rate)
        : scene_data(scene_data)
        , raytracer(get_program<RaytracerProgram>(compute_context.context,
                                                  compute_context.device),
                    compute_context.queue)
        , waveguide(get_program<RectangularProgram>(compute_context.context,
                                                    compute_context.device),
                    compute_context.queue,
                    MeshBoundary(scene_data),
                    mic,
                    waveguide_sample_rate)
        , source(source)
        , mic(mic)
        , waveguide_sample_rate(waveguide_sample_rate)
        , rays(rays)
        , impulses(impulses)
        //, output_sample_rate(output_sample_rate)
        , source_index(waveguide.get_index_for_coordinate(source))
        , mic_index(waveguide.get_index_for_coordinate(mic)) {
    if (!waveguide.inside(source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    if (!waveguide.inside(mic_index)) {
        throw std::runtime_error("mic is outside of mesh!");
    }
}

template <BufferType buffer_type>
typename WayverbEngine<buffer_type>::Intermediate
WayverbEngine<buffer_type>::run(const WayverbEngine::StateCallback& callback) {
    //  RAYTRACER  -----------------------------------------------------------//
    callback(State::starting_raytracer, 1.0);
    auto raytracer_step = 0;
    auto raytracer_results =
        raytracer.run(scene_data,
                      mic,
                      source,
                      rays,
                      impulses,
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

    auto input = waveguide_kernel(waveguide_sample_rate);

    //  If the max raytracer time is large this could take forever...
    auto waveguide_step = 0;
    auto waveguide_results =
        waveguide.init_and_run(corrected_source,
                               std::move(input),
                               mic_index,
                               steps,
                               [&callback, &waveguide_step, steps] {
                                   callback(State::running_waveguide,
                                            waveguide_step++ / (steps - 1.0));
                               });
    callback(State::finishing_waveguide, 1.0);

    auto output = std::vector<float>(waveguide_results.size());
    proc::transform(waveguide_results, output.begin(), [](const auto& i) {
        return i.pressure;
    });

    //  correct for filter time offset
    output.erase(output.begin(),
                 output.begin() + std::ceil(input.size() / 2.0));

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
