#include "image_source.h"

#include "raytracer/image_source/run.h"

#include "common/azimuth_elevation.h"
#include "common/string_builder.h"

#include "audio_file/audio_file.h"

image_source_test::image_source_test(
        const generic_scene_data<cl_float3, surface>& sd, float speed_of_sound)
        : voxelised_{sd, 5, padded(geo::get_aabb(sd), glm::vec3{0.1})}
        , speed_of_sound_{speed_of_sound} {}

audio image_source_test::operator()(const surface& surf,
                                    const glm::vec3& source,
                                    const model::receiver_settings& receiver) {
    voxelised_.set_surfaces(surf);

    const auto directions{get_random_directions(100000)};

    const auto sample_rate{44100.0};
    const auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<>>(
            directions.begin(),
            directions.end(),
            compute_context_,
            voxelised_,
            source,
            receiver.position)};

    const auto make_iterator{[=](auto i) {
        return raytracer::make_histogram_iterator(std::move(i),
                                                  speed_of_sound_);
    }};
    const auto histogram{
            raytracer::dirac_histogram(make_iterator(impulses.begin()),
                                       make_iterator(impulses.end()),
                                       sample_rate,
                                       20)};
    const auto img_src_results{mixdown(histogram.begin(), histogram.end())};

    static auto count{0};
    const auto fname{
            build_string("img_src_intensity_source_", count++, ".wav")};
    write(fname, make_audio_file(img_src_results, sample_rate), 16);
    const auto pressure{map_to_vector(img_src_results,
                                      [](auto i) { return std::sqrt(i); })};

    return {pressure, sample_rate, "image_source"};
}
