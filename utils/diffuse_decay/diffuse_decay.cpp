#include "raytracer/raytracer.h"

#include "audio_file/audio_file.h"

#include "utilities/string_builder.h"

auto produce_histogram(
        const voxelised_scene_data<cl_float3, surface>& voxelised,
        const model::parameters& params) {
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

    return raytracer::energy_histogram{histogram, histogram_sr};
}

int main() {
    constexpr model::parameters params{glm::vec3{-2, 0, 0}, glm::vec3{2, 0, 0}};

    const geo::box box{glm::vec3{-4}, glm::vec3{4}};
    constexpr auto absorption{0.1};
    constexpr auto scattering{0.1};

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, scattering)),
            2,
            0.1f)};

    const auto histogram{produce_histogram(voxelised, params)};

    const auto dim{dimensions(box)};
    const auto room_volume{dim.x * dim.y * dim.z};

    const auto speed_of_sound{340.0};

    const auto sample_rates = {4000.0, 8000.0, 16000.0, 32000.0};

    for (const auto& sample_rate : sample_rates) {
        std::cout << "sample rate: " << sample_rate << '\n';

        auto dirac_sequence{raytracer::generate_dirac_sequence(
                speed_of_sound, room_volume, sample_rate, 60.0)};

        {
            auto mono{dirac_sequence.sequence};
            normalize(mono);
            write(build_string("raw_dirac.", sample_rate, ".wav"),
                  audio_file::make_audio_file(mono, dirac_sequence.sample_rate),
                  16);
        }

        auto processed{raytracer::mono_diffuse_postprocessing(
                histogram, dirac_sequence, params.acoustic_impedance)};

        write(build_string("enveloped_dirac.", sample_rate, ".wav"),
              audio_file::make_audio_file(processed, sample_rate),
              16);

        const auto max_raytracer_amplitude{std::accumulate(
                begin(processed), end(processed), 0.0, [](auto i, double j) {
                    return std::max(i, std::abs(j));
                })};

        const auto direct{raytracer::get_direct(
                params.source, params.receiver, voxelised)};

        const auto direct_pressure{pressure_for_distance(
                direct->distance, params.acoustic_impedance)};

        const auto norm{std::max(max_raytracer_amplitude, direct_pressure)};

        for (auto& i : processed) {
            i /= norm;
        }

        write(build_string("normalized_enveloped_dirac.", sample_rate, ".wav"),
              audio_file::make_audio_file(processed, sample_rate),
              16);

        std::cout << "max raytracer amplitude: " << max_raytracer_amplitude
                  << '\n';
        std::cout << "direct pressure: " << direct_pressure << '\n';

        std::cout << '\n';
    }

    return EXIT_SUCCESS;
}
