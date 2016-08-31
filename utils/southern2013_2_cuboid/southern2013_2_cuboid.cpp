/// This test aims to replicate the cuboid test found in the southern2013_2
/// paper Room Impulse Response Synthesis and Validation Using a Hybrid Acoustic
/// Model.
/// This paper deals with a hybrid acoustic modelling method, and discusses a
/// number of verifications/tests for the model. I figure I should aim to get
/// similar results with my own model...

//  Modal analysis in a cuboid room
//  -------------------------------
//
//  Take a room with measurements 5.56 x 3.97 x 2.81
//  and sources/receivers at
//
//      s                       r
//  --- ----------------------- ------------------
//  0   {2.71, 1.89, 1.69}      {3.81, 1.13, 1.92}
//  1   {4.10, 1.39, 1.82}      {1.95, 0.60, 2.28}
//  2   {2.09, 2.12, 2.12}      {2.09, 3.08, 0.96}
//  3   {3.01, 2.68, 0.93}      {3.91, 1.89, 1.69}
//  4   {4.80, 2.18, 2.12}      {2.09, 0.99, 1.62}
//
//  set boundary materials to [0.8, 0.9, 0.999]
//
//  work out ideal modal frequencies
//
//  see whether results for [image source, diffuse tail, waveguide] match up

#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/decibels.h"
#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/write_audio_file.h"

#include <iostream>

struct audio final {
    aligned::vector<float> data;
    double sample_rate;
    std::string prefix;
};

constexpr auto speed_of_sound{340.0};
constexpr auto acoustic_impedance{400.0};

class test_case_harness final {
public:
    using single_test = std::function<audio(
            const surface&, const glm::vec3&, const model::ReceiverSettings&)>;

    //  Takes a scene (with a custom surface set), a source, and a receiver.
    //  Returns a signal representing the impulse response for a scene with
    //  these settings.
    void run_all_tests(single_test t) const {
        const auto max_tests{sources_.size() * receivers_.size() *
                             surfaces_.size()};
        auto count{0u};
        for (auto i{0u}; i != sources_.size(); ++i) {
            const auto& source{sources_[i]};
            for (auto j{0u}; j != receivers_.size(); ++j) {
                const auto& receiver{receivers_[j]};
                for (auto k{0u}; k != surfaces_.size(); ++k) {
                    const auto& surface{surfaces_[k]};
                    const auto results{t(surface,
                                         source,
                                         model::ReceiverSettings{receiver})};

                    const auto fname{build_string(results.prefix,
                                                  "_source_",
                                                  i,
                                                  "_receiver_",
                                                  j,
                                                  "_surface_",
                                                  k,
                                                  ".wav")};

                    snd::write(fname, {results.data}, results.sample_rate, 16);

                    count += 1;
                    std::cout << "finished: test " << count << " of "
                              << max_tests << '\n' << std::flush;
                }
            }
        }
    }

    scene_data get_scene_data() const { return scene_data_; }

private:
    geo::box box_{glm::vec3{0}, glm::vec3{5.56, 3.97, 2.81}};
    scene_data scene_data_{geo::get_scene_data(box_)};
    aligned::vector<glm::vec3> sources_{glm::vec3{2.71, 1.89, 1.69},
                                        glm::vec3{4.10, 1.39, 1.82},
                                        glm::vec3{2.09, 2.12, 2.12},
                                        glm::vec3{3.01, 2.68, 0.93},
                                        glm::vec3{4.80, 2.18, 2.12}};
    aligned::vector<glm::vec3> receivers_{glm::vec3{3.81, 1.13, 1.92},
                                          glm::vec3{1.95, 0.60, 2.28},
                                          glm::vec3{2.09, 3.08, 0.96},
                                          glm::vec3{3.91, 1.89, 1.69},
                                          glm::vec3{2.09, 0.99, 1.62}};
    aligned::vector<surface> surfaces_{make_surface(0.8, 0),
                                       make_surface(0.9, 0)};
};

//----------------------------------------------------------------------------//

class raytracer_test final {
public:
    raytracer_test(const scene_data& sd)
            : voxelised_{sd, 5, padded(sd.get_aabb(), glm::vec3{0.1})} {}

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::ReceiverSettings& receiver) {
        voxelised_.set_surfaces(surface);
        const auto specular{max(surface.specular)};
        const auto reflections{raytracer::compute_optimum_reflection_number(
                decibels::db2a(-60), specular)};
        progress_bar pb{std::cout, reflections};
        const auto results{raytracer::run(compute_context_,
                                          voxelised_,
                                          speed_of_sound,
                                          source,
                                          receiver.position,
                                          get_random_directions(100000),
                                          reflections,
                                          20,
                                          true,
                                          [&](auto i) { pb += 1; })};

        if (!results) {
            throw std::runtime_error("no results were generated by raytracer");
        }

        const auto sample_rate{44100.0};
        const auto output{raytracer::run_attenuation(
                compute_context_, receiver, *results, sample_rate, 400, 20)};

        if (output.size() != 1) {
            throw std::runtime_error("output should contain just one channel");
        }

        return {output.front(), sample_rate, "raytracer"};
    }

private:
    compute_context compute_context_{};
    voxelised_scene_data voxelised_;
};

//----------------------------------------------------------------------------//

class waveguide_test final {
public:
    waveguide_test(const scene_data& sd)
            : voxels_and_mesh_{
                      waveguide::compute_voxels_and_mesh(compute_context_,
                                                         sd,
                                                         glm::vec3{0},
                                                         sample_rate_,
                                                         speed_of_sound)} {}

    audio operator()(const surface& surface,
                     const glm::vec3& source,
                     const model::ReceiverSettings& receiver) {
        auto& mesh{std::get<1>(voxels_and_mesh_)};
        mesh.set_coefficients(waveguide::to_filter_coefficients(
                aligned::vector<struct surface>{surface}, sample_rate_));

        const auto input_node{compute_index(mesh.get_descriptor(), source)};
        const auto output_node{
                compute_index(mesh.get_descriptor(), receiver.position)};

        const aligned::vector<float> dry_signal{1};
        const auto input_signal{waveguide::make_transparent(dry_signal)};

        progress_bar pb{std::cout, input_signal.size()};
        const auto results{waveguide::run(compute_context_,
                                          mesh,
                                          input_node,
                                          input_signal,
                                          output_node,
                                          speed_of_sound,
                                          acoustic_impedance / speed_of_sound,
                                          [&](auto i) { pb += 1; })};

        const auto output{
                map_to_vector(results, [](auto i) { return i.pressure; })};

        return {output, sample_rate_, "waveguide"};
    }

private:
    compute_context compute_context_{};
    double sample_rate_{18000.0};
    std::tuple<voxelised_scene_data, waveguide::mesh> voxels_and_mesh_;
};

//----------------------------------------------------------------------------//

int main() {
    test_case_harness test_case_harness{};

    aligned::vector<test_case_harness::single_test> tests{
            raytracer_test{test_case_harness.get_scene_data()},
            waveguide_test{test_case_harness.get_scene_data()}};

    for (auto& i : tests) {
        test_case_harness.run_all_tests(i);
    }
}
