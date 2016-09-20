#include "waveguide.h"

#include "waveguide/make_transparent.h"
#include "waveguide/postprocessor/output_holder.h"
#include "waveguide/postprocessor/single_node.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/dc_blocker.h"
#include "common/dsp_vector_ops.h"
#include "common/frequency_domain_filter.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/write_audio_file.h"

#include <iostream>

waveguide_test::waveguide_test(const scene_data& sd,
                               float speed_of_sound,
                               float acoustic_impedance)
        : voxels_and_mesh_{waveguide::compute_voxels_and_mesh(compute_context_,
                                                              sd,
                                                              glm::vec3{0},
                                                              sample_rate_,
                                                              speed_of_sound)}
//, speed_of_sound_{speed_of_sound}
//, acoustic_impedance_{acoustic_impedance}
{}

audio waveguide_test::operator()(const surface& surface,
                                 const glm::vec3& source,
                                 const model::receiver_settings& receiver) {
    auto& voxels{std::get<0>(voxels_and_mesh_)};
    voxels.set_surfaces(surface);

    auto& mesh{std::get<1>(voxels_and_mesh_)};
    mesh.set_coefficients(waveguide::to_flat_coefficients(
            voxels.get_scene_data().get_surfaces()));

    const auto predicted_rt60{
            sabine_reverb_time(voxels.get_scene_data(), make_volume_type(0))
                    .s[0]};
    std::cout << "predicted rt60: " << predicted_rt60 << '\n' << std::flush;

    //  Find input and output nodes.
    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), receiver.position)};

    //  Create input signal.
    auto input_signal{
            waveguide::make_transparent(aligned::vector<float>{1})};
    input_signal.resize(sample_rate_ * predicted_rt60);

    auto prep{waveguide::preprocessor::make_single_soft_source(
            input_node, input_signal.begin(), input_signal.end())};

    waveguide::postprocessor::output_accumulator<
            waveguide::postprocessor::node_state>
            postprocessor{output_node};

    progress_bar pb{std::cerr, input_signal.size()};
    waveguide::run(compute_context_,
                   mesh,
                   prep,
                   [&](auto& a, const auto& b, auto c) {
                       postprocessor(a, b, c);
                       pb += 1;
                   },
                   true);

    //  Extract pressure signal.
    //  TODO via microphone simulation.
    auto pressure_signal{postprocessor.get_output()};

    {
        static auto count{0};
        auto copy{pressure_signal};
        normalize(copy);
        snd::write(build_string("pressure_signal_", count++, ".wav"),
                   {copy},
                   sample_rate_,
                   16);
    }

    //  Filter out everything above the waveguide nyquist.
    {
        fast_filter filter{pressure_signal.size() * 2};
        filter.filter(pressure_signal.begin(),
                      pressure_signal.end(),
                      pressure_signal.begin(),
                      [=](auto cplx, auto freq) {
                          //   We use quite a wide 'crossover' band here to
                          //   minimize ring - everything over 0.2 is pretty
                          //   much guaranteed to be garbage anyway.
                          return cplx *
                                 compute_lopass_magnitude(freq, 0.25, 0.1, 0);
                                 //compute_hipass_magnitude(freq,
                                 //                         hipass_cutoff,
                                 //                         hipass_cutoff / 2,
                                 //                         0);
                      });
    }

    {
        static auto count{0};
        auto copy{pressure_signal};
        normalize(copy);
        snd::write(build_string("nyquist_filtered_", count++, ".wav"),
                   {copy},
                   sample_rate_,
                   16);
    }

    //  Filter out dc component.

    {
        auto logged{map_to_vector(
                pressure_signal, [](auto i) { return std::log(i); })};
        {
            static auto count{0};
            snd::write(build_string("logged_", count++, ".wav"),
                       {logged},
                       sample_rate_,
                       16);
        }

        auto x{0ul};
        auto xy{map_to_vector(logged, [&x](auto y) {
            return glm::vec2{x++, y};
        })};

        const auto regression{
                simple_linear_regression(xy.begin() + xy.size() / 2, xy.end())};

        const auto b{std::exp(regression.c)};
        const auto a{regression.m};
        for (auto & i : xy) {
            i.y -= b * std::exp(i.x * a);
        }

        pressure_signal = map_to_vector(xy, [](auto i) {return i.y;});
    }

    {
        static auto count{0};
        auto copy{pressure_signal};
        normalize(copy);
        snd::write(build_string("dc_blocked_", count++, ".wav"),
                   {copy},
                   sample_rate_,
                   16);
    }

    return {pressure_signal, sample_rate_, "waveguide"};
}
