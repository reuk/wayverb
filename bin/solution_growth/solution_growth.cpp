#include "waveguide/boundary_adjust.h"
#include "waveguide/config.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/pcs.h"
#include "waveguide/postprocessor/node.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/preprocessor/soft_source.h"
#include "waveguide/waveguide.h"

#include "core/azimuth_elevation.h"
#include "core/callback_accumulator.h"
#include "core/cl/common.h"
#include "core/conversions.h"
#include "core/dc_blocker.h"
#include "core/filters_common.h"
#include "core/kernel.h"
#include "core/maximum_length_sequence.h"
#include "core/reverb_time.h"
#include "core/scene_data.h"
#include "core/serialize/surface.h"
#include "core/sinc.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/map_to_vector.h"
#include "utilities/progress_bar.h"

#include "audio_file/audio_file.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

constexpr auto speed_of_sound = 340.0;

util::aligned::vector<float> make_mls(size_t length) {
    const size_t order = std::floor(std::log(length) / std::log(2)) + 1;
    util::aligned::vector<float> ret;
    ret.reserve(std::pow(2, order));
    wayverb::core::generate_maximum_length_sequence(
            order, [&](auto value, auto /*step*/) { ret.emplace_back(value); });
    return ret;
}

struct make_hard_source_functor final {
    template <typename... Ts>
    auto operator()(Ts&&... ts) const {
        return wayverb::waveguide::preprocessor::make_hard_source(
                std::forward<Ts>(ts)...);
    }
};

struct make_soft_source_functor final {
    template <typename... Ts>
    auto operator()(Ts&&... ts) const {
        return wayverb::waveguide::preprocessor::make_soft_source(
                std::forward<Ts>(ts)...);
    }
};

int main(int /*argc*/, char** /*argv*/) {
    try {
        const glm::vec3 source{4.8, 2.18, 2.12};
        const glm::vec3 receiver{4.7, 2.08, 2.02};
        const auto sampling_frequency = 10000.0;

        constexpr auto absorption = 0.006;
        const auto surface =
                wayverb::core::make_surface<wayverb::core::simulation_bands>(
                        absorption, 0);

        const wayverb::core::compute_context cc{};
        const wayverb::core::geo::box boundary{glm::vec3{0},
                                               glm::vec3{5.56, 3.97, 2.81}};

        const auto scene_data =
                wayverb::core::geo::get_scene_data(boundary, surface);

        const auto rt60 = max_element(sabine_reverb_time(scene_data, 0));

        const size_t simulation_length =
                std::ceil(sampling_frequency * rt60) / 2;
        std::cout << "simulation steps: " << simulation_length << '\n';

        auto voxels_and_mesh = wayverb::waveguide::compute_voxels_and_mesh(
                cc, scene_data, receiver, sampling_frequency, speed_of_sound);

        voxels_and_mesh.mesh.set_coefficients(
                wayverb::waveguide::to_flat_coefficients(absorption));

        const auto& model = voxels_and_mesh.mesh;

        const auto nodes = model.get_descriptor().dimensions.s[0] *
                           model.get_descriptor().dimensions.s[1] *
                           model.get_descriptor().dimensions.s[2];
        std::cout << "nodes: " << nodes << '\n';

        const auto receiver_index =
                compute_index(model.get_descriptor(), receiver);
        const auto source_index = compute_index(model.get_descriptor(), source);

        if (!wayverb::waveguide::is_inside(model, receiver_index)) {
            throw std::runtime_error("receiver is outside of mesh!");
        }
        if (!wayverb::waveguide::is_inside(model, source_index)) {
            throw std::runtime_error("source is outside of mesh!");
        }

        struct signal final {
            std::string name;
            util::aligned::vector<float> kernel;
        };

        const auto valid_portion = 0.1;

        const auto pcs_sig =
                wayverb::waveguide::design_pcs_source(4096,
                                                      400,
                                                      speed_of_sound,
                                                      sampling_frequency,
                                                      0.05,
                                                      0.025,
                                                      100,
                                                      0.7)
                        .signal;

        util::aligned::vector<signal> signals{
                {"dirac", util::aligned::vector<float>{1.0}},
                {"sin_modulated_gaussian",
                 wayverb::core::kernels::gen_sin_modulated_gaussian(
                         valid_portion / 2)},
                {"differentiated_gaussian",
                 wayverb::core::kernels::gen_gaussian_dash(valid_portion /
                 2)},
                {"ricker",
                 wayverb::core::kernels::gen_ricker(valid_portion / 2)},
                {"pcs",
                 util::map_to_vector(begin(pcs_sig),
                                     end(pcs_sig),
                                     [](auto s) -> float { return s; })},
        };

        const auto do_sim = [&](const auto& signal,
                                const auto& name,
                                const auto& type_string,
                                const auto& callback) {
            auto kernel = signal;
            kernel.resize(simulation_length);

            write(util::build_string("solution_growth.",
                                     name,
                                     '.',
                                     type_string,
                                     ".input.aif")
                          .c_str(),
                  kernel,
                  sampling_frequency,
                  audio_file::format::aif,
                  audio_file::bit_depth::float32);

            auto prep = callback(source_index, kernel.begin(), kernel.end());

            wayverb::core::callback_accumulator<
                    wayverb::waveguide::postprocessor::node>
                    postprocessor{receiver_index};

            util::progress_bar pb;
            wayverb::waveguide::run(cc,
                                    model,
                                    prep,
                                    [&](auto& a, const auto& b, auto c) {
                                        postprocessor(a, b, c);
                                        set_progress(pb, c, kernel.size());
                                    },
                                    true);

            const auto output = postprocessor.get_output();

            write(util::build_string("solution_growth.",
                                     name,
                                     '.',
                                     type_string,
                                     ".output.aif")
                          .c_str(),
                  output,
                  sampling_frequency,
                  audio_file::format::aif,
                  audio_file::bit_depth::float32);
        };

        for (const auto& i : signals) {
            do_sim(i.kernel, i.name, "hard", make_hard_source_functor{});

            do_sim(i.kernel, i.name, "soft", make_soft_source_functor{});

            const auto transparent = wayverb::waveguide::make_transparent(
                    i.kernel.data(), i.kernel.data() + i.kernel.size());
            do_sim(transparent,
                   i.name,
                   "transparent",
                   make_soft_source_functor{});
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "critical runtime error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
