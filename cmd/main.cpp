//  project internal
#include "waveguide.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"
#include "microphone.h"
#include "hrtf_attenuator.h"
#include "combined_config.h"

#include "rayverb.h"

#include "cl_common.h"

//  dependency
#include "logger.h"
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"
#include "samplerate.h"

#include <gflags/gflags.h>

//  stdlib
#include <random>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>

// -1 <= z <= 1, -pi <= theta <= pi
cl_float3 spherePoint(float z, float theta) {
    const float ztemp = sqrtf(1 - z * z);
    return (cl_float3){{ztemp * cosf(theta), ztemp * sinf(theta), z, 0}};
}

std::vector<cl_float3> getRandomDirections(unsigned long num) {
    std::vector<cl_float3> ret(num);
    std::uniform_real_distribution<float> zDist(-1, 1);
    std::uniform_real_distribution<float> thetaDist(-M_PI, M_PI);
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);

    for (auto& i : ret)
        i = spherePoint(zDist(engine), thetaDist(engine));

    return ret;
}

double a2db(double a) {
    return 20 * log10(a);
}

double db2a(double db) {
    return pow(10, db / 20);
}

std::vector<float> squintegrate(const std::vector<float>& sig) {
    std::vector<float> ret(sig.size());
    partial_sum(sig.rbegin(),
                sig.rend(),
                ret.rbegin(),
                [](auto i, auto j) { return i + j * j; });
    return ret;
}

int rt60(const std::vector<float>& sig) {
    auto squintegrated = squintegrate(sig);
    normalize(squintegrated);
    auto target = db2a(-60);
    return distance(squintegrated.begin(),
                    find_if(squintegrated.begin(),
                            squintegrated.end(),
                            [target](auto i) { return i < target; }));
}

std::vector<float> exponential_decay_envelope(int steps,
                                              float attenuation_factor) {
    std::vector<float> ret(steps);
    auto amp = 1.0f;
    generate(begin(ret),
             end(ret),
             [&amp, attenuation_factor] {
                 auto t = amp;
                 amp *= attenuation_factor;
                 return t;
             });
    return ret;
}

int main(int argc, char** argv) {
    Logger::restart();
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc != 5) {
        Logger::log_err(
            "expecting a config file, an input model, an input material file, "
            "and an output filename");

        Logger::log_err("actually found: ");
        for (auto i = 0u; i != argc; ++i) {
            Logger::log_err("arg ", i, ": ", argv[i]);
        }

        return EXIT_FAILURE;
    }

    std::string config_file = argv[1];
    std::string model_file = argv[2];
    std::string material_file = argv[3];
    std::string output_file = argv[4];

    auto context = get_context();
    auto device = get_device(context);
    cl::CommandQueue queue(context, device);

    CombinedConfig cc;
    try {
        cc = read_config(config_file);
    } catch (const std::runtime_error& e) {
        Logger::log_err("config load error: ", e.what());
        return EXIT_FAILURE;
    }

    Logger::log("mic ", cc.get_mic());
    Logger::log("source ", cc.get_source());

    unsigned long format, depth;

    try {
        format = get_file_format(output_file);
        depth = get_file_depth(cc.get_bit_depth());
    } catch (const std::runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    }

    auto directions = getRandomDirections(cc.get_rays());

    try {
        SceneData scene_data(model_file, material_file);

        MeshBoundary boundary(scene_data);
        auto waveguide_program =
            get_program<TetrahedralProgram>(context, device);
        TetrahedralWaveguide waveguide(waveguide_program,
                                       queue,
                                       boundary,
                                       cc.get_divisions(),
                                       cc.get_mic());
        auto mic_index = waveguide.get_index_for_coordinate(cc.get_mic());
        auto source_index = waveguide.get_index_for_coordinate(cc.get_source());

        auto corrected_mic = waveguide.get_coordinate_for_index(mic_index);
        auto corrected_source =
            waveguide.get_coordinate_for_index(source_index);

        Logger::log("mic: ", cc.get_mic());
        Logger::log("corrected: ", corrected_mic);

#ifdef TESTING
        auto steps = 1 << 8;
#else
        auto steps = 1 << 13;
#endif

#if 0
        auto raytrace_program = get_program<RayverbProgram>(context, device);
        Raytrace raytrace(raytrace_program, queue, num_impulses, scene_data);
        raytrace.raytrace(
            convert(corrected_mic), convert(corrected_source), directions);
        auto results = raytrace.getAllRaw(false);
        std::vector<Speaker> speakers{Speaker{cl_float3{{0, 0, 0}}, 0}};
        auto attenuated =
            Attenuate(raytrace_program, queue).attenuate(results, speakers);

        //  TODO ensure outputs are properly aligned
        //  fixPredelay(attenuated);

        auto flattened = flattenImpulses(attenuated, output_sr);
        auto raytrace_results = process(FILTER_TYPE_BIQUAD_ONEPASS,
                                        flattened,
                                        output_sr,
                                        true,
                                        ray_hipass,
                                        true,
                                        1.0);
        normalize(raytrace_results);

        write_sndfile(output_file + ".raytrace.full.wav",
                      raytrace_results,
                      output_sr,
                      depth,
                      format);

        LinkwitzRiley hipass;
        hipass.setParams(filter_freq, output_sr * 0.45, output_sr);
        for (auto & i : raytrace_results)
            hipass.filter(i);
        normalize(raytrace_results);

        write_sndfile(output_file + ".raytrace.hipass.wav",
                      raytrace_results,
                      output_sr,
                      depth,
                      format);

        auto decay_frames = rt60(raytrace_results.front());
        auto attenuation_factor = pow(db2a(-60), 1.0 / decay_frames);
        attenuation_factor = sqrt(attenuation_factor);
#else
        auto attenuation_factor = pow(db2a(-60), 1.0 / steps);
#endif

        auto w_results = waveguide.run_gaussian(
            cc.get_source(), mic_index, steps, cc.get_waveguide_sample_rate());

        Microphone microphone(Vec3f(0, 0, 1), 0.5);
        HrtfAttenuator hrtf_attenuator(
            Vec3f(0, 0, 1), Vec3f(0, 1, 0), 0, cc.get_waveguide_sample_rate());
        //        auto w_pressures = microphone.process(w_results);
        auto w_pressures = hrtf_attenuator.process(w_results);

        normalize(w_pressures);

        std::vector<float> out_signal(cc.get_output_sample_rate() *
                                      w_results.size() /
                                      cc.get_waveguide_sample_rate());

        SRC_DATA sample_rate_info{w_pressures.data(),
                                  out_signal.data(),
                                  long(w_results.size()),
                                  long(out_signal.size()),
                                  0,
                                  0,
                                  0,
                                  cc.get_output_sample_rate() /
                                      double(cc.get_waveguide_sample_rate())};

        src_simple(&sample_rate_info, SRC_SINC_BEST_QUALITY, 1);

        auto envelope =
            exponential_decay_envelope(out_signal.size(), attenuation_factor);
        elementwise_multiply(out_signal, envelope);

        write_sndfile(output_file + ".waveguide.full.wav",
                      {out_signal},
                      cc.get_output_sample_rate(),
                      depth,
                      format);

        LinkwitzRiley lopass;
        lopass.setParams(
            1, cc.get_filter_frequency(), cc.get_output_sample_rate());
        lopass.filter(out_signal);

        normalize(out_signal);

        std::vector<std::vector<float>> waveguide_results = {out_signal};

        write_sndfile(output_file + ".waveguide.lopass.wav",
                      waveguide_results,
                      cc.get_output_sample_rate(),
                      depth,
                      format);

        auto raytrace_amp = 0.95;
        auto waveguide_amp = 0.05;

#if 0
        auto max_index = max(raytrace_results.front().size(),
                             waveguide_results.front().size());
        std::vector<float> summed_results(max_index, 0);
        for (auto i = 0u; i != max_index; ++i) {
            if (i < raytrace_results.front().size())
                summed_results[i] += raytrace_amp * raytrace_results.front()[i];
            if (i < waveguide_results.front().size())
                summed_results[i] +=
                    waveguide_amp * waveguide_results.front()[i];
        }
        normalize(summed_results);
        write_sndfile(output_file + ".summed.wav",
                      {summed_results},
                      output_sr,
                      depth,
                      format);
#endif

    } catch (const cl::Error& e) {
        Logger::log_err("critical cl error: ", e.what());
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        Logger::log_err("critical runtime error: ", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log_err("unknown error");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
