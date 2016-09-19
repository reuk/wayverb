#include "waveguide/config.h"
#include "waveguide/filters.h"
#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/microphone.h"
#include "waveguide/postprocessor/single_node.h"
#include "waveguide/preprocessor/single_soft_source.h"
#include "waveguide/program.h"
#include "waveguide/setup.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/cl/common.h"
#include "common/progress_bar.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/write_audio_file.h"

#include "gtest/gtest.h"

#include <random>

static constexpr auto samplerate{44100.0};

TEST(peak_filter_coefficients, peak_filter_coefficients) {
    static std::default_random_engine engine{std::random_device()()};
    static std::uniform_real_distribution<cl_float> range{0, samplerate / 2};
    for (auto i = 0; i != 10; ++i) {
        const auto descriptor =
                waveguide::filter_descriptor{0, range(engine), 1.414};
        const auto coefficients =
                waveguide::get_peak_coefficients(descriptor, samplerate);

        ASSERT_TRUE(proc::equal(coefficients.b, std::begin(coefficients.a)));
    }
}

TEST(run_waveguide, run_waveguide) {
    const auto steps{700};

    const compute_context cc{};

    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    constexpr glm::vec3 source{2, 1.5, 1};

    aligned::vector<glm::vec3> receivers{
            {2, 1.5, 2}, {2, 1.5, 3}, {2, 1.5, 4}, {2, 1.5, 5}};

    //  init simulation parameters

    const auto scene_data{geo::get_scene_data(box)};

    constexpr auto speed_of_sound{340.0};
    const auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc, scene_data, source, samplerate, speed_of_sound)};

    auto model{std::get<1>(voxels_and_mesh)};
    model.set_coefficients(waveguide::to_flat_coefficients(
            aligned::vector<surface>{make_surface(0.01, 0)}));

    //  get a waveguide

    const auto source_index{compute_index(model.get_descriptor(), source)};

    if (!waveguide::is_inside(model, source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    class output_holder final {
    public:
        output_holder(size_t node)
                : node_{node} {}

        waveguide::step_postprocessor get_postprocessor() const {
            return waveguide::postprocessor::node{
                    node_, [&](auto i) { output_.emplace_back(i); }};
        }

        const auto& get_output() const { return output_; }

    private:
        mutable aligned::vector<float> output_;
        size_t node_;
    };

    auto input{waveguide::make_transparent(aligned::vector<float>{1})};
    input.resize(steps);

    auto prep{waveguide::preprocessor::make_single_soft_source(
            source_index, input.begin(), input.end())};

    aligned::vector<output_holder> output_holders;
    output_holders.reserve(receivers.size());
    for (const auto& receiver : receivers) {
        const auto receiver_index{
                compute_index(model.get_descriptor(), receiver)};
        if (!waveguide::is_inside(model, receiver_index)) {
            throw std::runtime_error("receiver is outside of mesh!");
        }
        output_holders.emplace_back(receiver_index);
    }

    progress_bar pb{std::cout, steps};
    auto callback_counter{0};
    const auto completed_steps{waveguide::run(
            cc,
            model,
            input.size(),
            prep,
            map_to_vector(output_holders,
                          [](auto& i) { return i.get_postprocessor(); }),
            [&](auto i) {
                pb += 1;
                ASSERT_EQ(i, callback_counter++);
            },
            true)};

    ASSERT_EQ(completed_steps, input.size());

    auto count{0ul};
    for (const auto& output_holder : output_holders) {
        snd::write(build_string("waveguide_receiver_", count++, ".wav"),
                   {output_holder.get_output()},
                   samplerate,
                   16);
    }

    const auto max_values{
            map_to_vector(output_holders, [](const auto& output_holder) {
                const auto output{output_holder.get_output()};
                const auto begin{proc::find_if(output, [](auto samp) {
                    return 0.0001 < std::abs(samp);
                })};
                const auto end{
                        begin +
                        std::min(std::distance(begin, output.end()), 100l)};
                if (begin == end) {
                    return 0.0f;
                }
                return *std::max_element(begin, end);
            })};

    for (auto val : max_values) {
        std::cout << "value: " << val << '\n';
    }
}
