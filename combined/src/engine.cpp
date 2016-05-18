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

WayverbEngine::WayverbEngine(ComputeContext& compute_context,
                             const MeshBoundary& boundary,
                             const Vec3f& source,
                             const Vec3f& mic,
                             float waveguide_sample_rate,
                             int rays,
                             int impulses,
                             float output_sample_rate)
        : raytracer(get_program<RaytracerProgram>(compute_context.context,
                                                  compute_context.device),
                    compute_context.queue)
        , waveguide(get_program<RectangularProgram>(compute_context.context,
                                                    compute_context.device),
                    compute_context.queue,
                    boundary,
                    mic,
                    waveguide_sample_rate)
        , source(source)
        , mic(mic)
        , waveguide_sample_rate(waveguide_sample_rate)
        /*
        , rays(rays)
        , impulses(impulses)
        , output_sample_rate(output_sample_rate)
        */
        , source_index(waveguide.get_index_for_coordinate(source))
        , mic_index(waveguide.get_index_for_coordinate(mic)) {

    if (! waveguide.inside(source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    if (! waveguide.inside(mic_index)) {
        throw std::runtime_error("mic is outside of mesh!");
    }
}

WayverbEngine::Intermediate WayverbEngine::run(
    const WayverbEngine::StateCallback& callback) {

    //  RAYTRACER  -----------------------------------------------------------//

    //  TODO calculate this properly
    auto steps = 16000;

    //  WAVEGUIDE  -----------------------------------------------------------//
    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    auto input = waveguide_kernel(waveguide_sample_rate);

    auto step = 0;
    auto results = waveguide.init_and_run(corrected_source,
                                          std::move(input),
                                          mic_index,
                                          steps,
                                          [&callback, &step, steps] {
                                              callback(State::running_waveguide,
                                                       step++ / (steps - 1.0));
                                          });

    auto output = std::vector<float>(results.size());
    proc::transform(
        results, output.begin(), [](const auto& i) { return i.pressure; });

    //  correct for filter time offset
    output.erase(output.begin(),
                 output.begin() + std::ceil(input.size() / 2.0));

    Intermediate ret;
    return ret;
}

std::vector<std::vector<float>> WayverbEngine::attenuate(
    const Intermediate& i,
    //  other args or whatever
    const StateCallback& callback) {
    std::vector<std::vector<float>> ret;
    return ret;
}
