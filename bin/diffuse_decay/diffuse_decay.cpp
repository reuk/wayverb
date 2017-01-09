#include "raytracer/histogram.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/raytracer.h"
#include "raytracer/stochastic/finder.h"
#include "raytracer/stochastic/postprocess.h"
#include "raytracer/stochastic/postprocessing.h"

#include "audio_file/audio_file.h"

#include "utilities/string_builder.h"

auto produce_histogram(
        const wayverb::core::voxelised_scene_data<
                cl_float3,
                wayverb::core::surface<wayverb::core::simulation_bands>>&
                voxelised,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const wayverb::core::environment& environment) {
    const wayverb::core::compute_context cc{};

    const auto rays = 1 << 16;

    const wayverb::core::scene_buffers buffers{cc.context, voxelised};

    constexpr auto receiver_radius = 0.1f;
    constexpr auto histogram_sr = 1000.0f;

    wayverb::raytracer::stochastic::finder finder{
            cc,
            rays,
            source,
            receiver,
            receiver_radius,
            wayverb::raytracer::stochastic::compute_ray_energy(
                    rays, source, receiver, receiver_radius)};
    util::aligned::vector<wayverb::core::bands_type> histogram;

    const auto make_ray_iterator = [&](auto it) {
        return util::make_mapping_iterator_adapter(
                std::move(it), [&](const auto& i) {
                    return wayverb::core::geo::ray{source, i};
                });
    };

    std::default_random_engine engine{std::random_device{}()};

    wayverb::raytracer::reflector ref{
            cc,
            receiver,
            make_ray_iterator(wayverb::raytracer::
                                      make_random_direction_generator_iterator(
                                              0, engine)),
            make_ray_iterator(wayverb::raytracer::
                                      make_random_direction_generator_iterator(
                                              rays, engine))};

    for (auto i = 0ul; i != 100; ++i) {
        const auto reflections = ref.run_step(buffers);

        const auto output =
                finder.process(begin(reflections), end(reflections), buffers);
        const auto to_histogram = [&](auto& in) {
            const auto make_iterator = [&](auto it) {
                return wayverb::raytracer::make_histogram_iterator(
                        std::move(it), environment.speed_of_sound);
            };

            incremental_histogram(histogram,
                                  make_iterator(begin(in)),
                                  make_iterator(end(in)),
                                  histogram_sr,
                                  wayverb::raytracer::dirac_sum_functor{});
        };
        to_histogram(output.stochastic);
        to_histogram(output.specular);
    }

    return wayverb::raytracer::stochastic::energy_histogram{histogram_sr,
                                                            histogram};
}

int main() {
    constexpr auto source = glm::vec3{-2, 0, 0}, receiver = glm::vec3{2, 0, 0};

    const wayverb::core::geo::box box{glm::vec3{-4}, glm::vec3{4}};
    const wayverb::core::environment environment{};
    constexpr auto absorption = 0.1;
    constexpr auto scattering = 0.1;

    const auto voxelised = make_voxelised_scene_data(
            wayverb::core::geo::get_scene_data(
                    box,
                    wayverb::core::make_surface<
                            wayverb::core::simulation_bands>(absorption,
                                                             scattering)),
            2,
            0.1f);

    const auto histogram =
            produce_histogram(voxelised, source, receiver, environment);

    const auto dim = dimensions(box);
    const auto room_volume = dim.x * dim.y * dim.z;

    const auto speed_of_sound = 340.0;

    const auto sample_rates = {4000.0, 8000.0, 16000.0, 32000.0};

    for (const auto& sample_rate : sample_rates) {
        std::cout << "sample rate: " << sample_rate << '\n';

        auto dirac_sequence =
                wayverb::raytracer::stochastic::generate_dirac_sequence(
                        speed_of_sound, room_volume, sample_rate, 60.0);

        {
            auto mono = dirac_sequence.sequence;
            wayverb::core::normalize(mono);
            write(util::build_string("raw_dirac.", sample_rate, ".wav").c_str(),
                  mono,
                  dirac_sequence.sample_rate,
                  audio_file::format::wav,
                  audio_file::bit_depth::pcm16);
        }

        auto processed = wayverb::raytracer::stochastic::postprocessing(
                histogram, dirac_sequence, environment.acoustic_impedance);

        write(util::build_string("enveloped_dirac.", sample_rate, ".wav")
                      .c_str(),
              processed,
              sample_rate,
              audio_file::format::wav,
              audio_file::bit_depth::pcm16);

        const auto max_raytracer_amplitude = std::accumulate(
                begin(processed), end(processed), 0.0, [](auto i, double j) {
                    return std::max(i, std::abs(j));
                });

        const auto direct = wayverb::raytracer::image_source::get_direct(
                source, receiver, voxelised);

        const auto direct_pressure = wayverb::core::pressure_for_distance(
                direct->distance, environment.acoustic_impedance);

        const auto norm = std::max(max_raytracer_amplitude, direct_pressure);

        for (auto& i : processed) {
            i /= norm;
        }

        write(util::build_string(
                      "normalized_enveloped_dirac.", sample_rate, ".wav")
                      .c_str(),
              processed,
              sample_rate,
              audio_file::format::wav,
              audio_file::bit_depth::pcm16);

        std::cout << "max raytracer amplitude: " << max_raytracer_amplitude
                  << '\n';
        std::cout << "direct pressure: " << direct_pressure << '\n';

        std::cout << '\n';
    }

    return EXIT_SUCCESS;
}
