#include "image_source.h"

#include "raytracer/image_source/run.h"

#include "common/azimuth_elevation.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

image_source_test::image_source_test(
        const generic_scene_data<cl_float3, surface>& sd,
        float speed_of_sound,
        float acoustic_impedance)
        : voxelised_{sd, 5, padded(geo::get_aabb(sd), glm::vec3{0.1})}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance} {}

audio image_source_test::operator()(const surface& surface,
                                    const glm::vec3& source,
                                    const model::receiver_settings& receiver) {
    voxelised_.set_surfaces(surface);

    const auto directions{get_random_directions(100000)};

    const auto sample_rate{44100.0};
    const auto impulses{raytracer::image_source::run<
            impulse,
            raytracer::image_source::fast_pressure_calculator>(
            directions.begin(),
            directions.end(),
            compute_context_,
            voxelised_,
            source,
            receiver.position,
            speed_of_sound_,
            acoustic_impedance_,
            sample_rate)};

    const auto img_src_results{
            mixdown(raytracer::dirac_histogram(impulses.begin(),
                                               impulses.end(),
                                               speed_of_sound_,
                                               sample_rate,
                                               20))};

    static auto count{0};
    const auto fname{
            build_string("img_src_intensity_source_", count++, ".wav")};
    snd::write(fname, {img_src_results}, sample_rate, 16);
    const auto pressure{map_to_vector(img_src_results,
                                      [](auto i) { return std::sqrt(i); })};

    return {pressure, sample_rate, "image_source"};
}
