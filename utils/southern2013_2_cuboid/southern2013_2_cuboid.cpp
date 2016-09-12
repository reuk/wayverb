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

#include "image_source.h"
#include "img_src_and_waveguide.h"
#include "waveguide.h"

#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/run.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "waveguide/make_transparent.h"
#include "waveguide/mesh.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/decibels.h"
#include "common/progress_bar.h"
#include "common/reverb_time.h"
#include "common/schroeder.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/write_audio_file.h"

#include <iostream>

class test_case_harness final {
public:
    using single_test = std::function<audio(
            const surface&, const glm::vec3&, const model::receiver_settings&)>;

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

                    auto sd{scene_data_};
                    sd.set_surfaces(surface);

                    const auto results{t(surface,
                                         source,
                                         model::receiver_settings{receiver})};

                    const auto fname{build_string(results.prefix,
                                                  "_source_",
                                                  i,
                                                  "_receiver_",
                                                  j,
                                                  "_surface_",
                                                  k,
                                                  ".wav")};

                    snd::write(fname, {results.data}, results.sample_rate, 16);

                    const auto measured_rt20{rt20(results.data) /
                                             results.sample_rate};
                    const auto measured_rt30{rt30(results.data) /
                                             results.sample_rate};

                    std::cout << "rt20: " << measured_rt20 << std::endl;
                    std::cout << "rt30: " << measured_rt30 << std::endl;
                    const auto measured_rt60{
                            (measured_rt20 * 3 + measured_rt30 * 2) / 2};
                    std::cout << "measured rt60 is around " << measured_rt60
                              << '\n';

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
    aligned::vector<glm::vec3> sources_{
            // glm::vec3{2.71, 1.89, 1.69},
            // glm::vec3{4.10, 1.39, 1.82},
            // glm::vec3{2.09, 2.12, 2.12},
            // glm::vec3{3.01, 2.68, 0.93},
            glm::vec3{4.80, 2.18, 2.12},
    };
    aligned::vector<glm::vec3> receivers_{
            // glm::vec3{3.81, 1.13, 1.92},
            // glm::vec3{1.95, 0.60, 2.28},
            // glm::vec3{2.09, 3.08, 0.96},
            glm::vec3{3.91, 1.89, 1.69},
            // glm::vec3{2.09, 0.99, 1.62},
    };
    aligned::vector<surface> surfaces_{make_surface(1 - pow(0.99, 2), 0),
                                       make_surface(1 - pow(0.9, 2), 0),
                                       make_surface(1 - pow(0.8, 2), 0)};
};

//----------------------------------------------------------------------------//

int main() {
    constexpr auto speed_of_sound{340.0};
    constexpr auto acoustic_impedance{400.0};

    test_case_harness test_case_harness{};

    aligned::vector<test_case_harness::single_test> tests{
            img_src_and_waveguide_test{test_case_harness.get_scene_data(),
                                       speed_of_sound,
                                       acoustic_impedance},
            // image_source_test{test_case_harness.get_scene_data(),
            //                  speed_of_sound,
            //                  acoustic_impedance},
            // waveguide_test{test_case_harness.get_scene_data(),
            //               speed_of_sound,
            //               acoustic_impedance},
    };

    for (auto& i : tests) {
        test_case_harness.run_all_tests(i);
    }
}
