#include "raytracer/raytracer.h"

#include "audio_file/audio_file.h"

auto produce_histogram(const geo::box& box, float acoustic_impedance) {
    constexpr auto absorption{0.1};
    constexpr auto scattering{0.1};

    constexpr model::parameters params{glm::vec3{-2, 0, 0}, glm::vec3{2, 0, 0}};

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, scattering)),
            2,
            0.1f)};

    const compute_context cc{};

    const auto directions{get_random_directions(1 << 16)};

    const scene_buffers buffers{cc.context, voxelised};

    constexpr auto receiver_radius{0.1f};
    constexpr auto histogram_sr{1000.0f};

    raytracer::diffuse::finder finder{
            cc, params, receiver_radius, directions.size()};
    aligned::vector<volume_type> histogram;

    const auto make_ray_iterator{[&](auto it) {
        return make_mapping_iterator_adapter(std::move(it), [&](const auto& i) {
            return geo::ray{params.source, i};
        });
    }};

    raytracer::reflector ref{cc,
                             params.receiver,
                             make_ray_iterator(begin(directions)),
                             make_ray_iterator(end(directions))};

    for (auto i{0ul}; i != 100; ++i) {
        const auto reflections{ref.run_step(buffers)};

        const auto output{
                finder.process(begin(reflections), end(reflections), buffers)};
        const auto to_histogram{[&](auto& in) {
            const auto make_iterator{[&](auto it) {
                return raytracer::make_histogram_iterator(
                        std::move(it), params.speed_of_sound);
            }};
            constexpr auto max_time{60.0};
            incremental_histogram(histogram,
                                  make_iterator(begin(in)),
                                  make_iterator(end(in)),
                                  histogram_sr,
                                  max_time,
                                  raytracer::dirac_sum_functor{});
        }};
        to_histogram(output.diffuse);
        to_histogram(output.specular);
    }

    /*
    const auto max_hist{
            std::accumulate(begin(histogram),
                            end(histogram),
                            volume_type{},
                            [](auto i, auto j) { return max(i, j); })};
    const auto max_energy{max_element(max_hist) /
                          pressure_to_intensity(1, acoustic_impedance)};

    for (auto& i : histogram) {
        i /= max_energy;
    }
    */

    return raytracer::energy_histogram{histogram, histogram_sr};
}

int main() {
    const geo::box box{glm::vec3{-4}, glm::vec3{4}};
    const auto acoustic_impedance{400.0f};

    const auto histogram{produce_histogram(box, acoustic_impedance)};

    const auto dim{dimensions(box)};
    const auto room_volume{dim.x * dim.y * dim.z};

    const auto speed_of_sound{340.0};

    const auto sample_rate{16000.0};

    auto dirac_sequence{raytracer::prepare_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, 60.0)};

    {
        auto mono{mixdown(begin(dirac_sequence.sequence),
                          end(dirac_sequence.sequence))};
        normalize(mono);
        write("filtered_dirac.wav",
              audio_file::make_audio_file(mono, dirac_sequence.sample_rate),
              16);
    }

    auto processed{raytracer::mono_diffuse_postprocessing(
            histogram, dirac_sequence, acoustic_impedance)};

    {
        write("enveloped_dirac.wav",
              audio_file::make_audio_file(processed, sample_rate),
              16);
        normalize(processed);
        write("normalized_enveloped_dirac.wav",
              audio_file::make_audio_file(processed, sample_rate),
              16);
    }

    return EXIT_SUCCESS;
}
