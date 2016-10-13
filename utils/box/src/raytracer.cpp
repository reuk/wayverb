#include "box/raytracer.h"

#include "raytracer/raytracer.h"

#include "common/dsp_vector_ops.h"

#include "audio_file/audio_file.h"

aligned::vector<impulse<8>> run_raytracer(const geo::box& box,
                                          float absorption,
                                          float scattering,
                                          const glm::vec3& source,
                                          const glm::vec3& receiver,
                                          float acoustic_impedance,
                                          bool flip_phase) {
    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, scattering)),
            2,
            0.1f)};

    const compute_context cc{};

    const auto directions{get_random_directions(1 << 16)};

    auto results{raytracer::run(begin(directions),
                                end(directions),
                                cc,
                                voxelised,
                                source,
                                receiver,
                                0,
                                flip_phase,
                                true,
                                [](auto i) {})};

    if (!results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    for (auto it{results->audio.begin()}, end{results->audio.end()}; it != end;
         ++it) {
        it->volume *= pressure_for_distance(it->distance, acoustic_impedance);
    }

    {
        const auto speed_of_sound{340.0};
        const auto sample_rate{44100.0};
        const auto b_impulses{results->audio.diffuse_begin()},
                e_impulses{results->audio.diffuse_end()};
        auto processed{raytracer::postprocess(b_impulses,
                                              e_impulses,
                                              receiver,
                                              speed_of_sound,
                                              sample_rate,
                                              20.0)};
        normalize(processed);
        write("diffuse_only.wav",
              audio_file::make_audio_file(processed, sample_rate),
              16);
    }

    return std::move(results->audio.get_data());
}
