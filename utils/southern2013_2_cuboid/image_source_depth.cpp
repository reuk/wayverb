#include "image_source_depth.h"

#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "common/aligned/map.h"
#include "common/azimuth_elevation.h"
#include "common/dsp_vector_ops.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

image_source_depth_test::image_source_depth_test(
        const generic_scene_data<cl_float3, surface>& sd,
        float speed_of_sound,
        float acoustic_impedance)
        : voxelised_{sd, 5, padded(geo::get_aabb(sd), glm::vec3{0.1})}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance} {}

audio image_source_depth_test::operator()(
        const surface& surf,
        const glm::vec3& source,
        const model::receiver_settings& receiver) {
    voxelised_.set_surfaces(surf);

    const auto sample_rate{44100.0};

    const auto directions{get_random_directions(10000)};

    using inner_callback = raytracer::image_source::depth_counter_calculator<
            raytracer::image_source::fast_pressure_calculator<surface>>;

    auto impulses{
            raytracer::image_source::run<inner_callback>(directions.begin(),
                                                         directions.end(),
                                                         compute_context_,
                                                         voxelised_,
                                                         source,
                                                         receiver.position,
                                                         speed_of_sound_)};

    aligned::map<size_t, aligned::vector<impulse<8>>> impulses_by_depth{};
    for (const auto& i : impulses) {
        impulses_by_depth[i.depth].emplace_back(i.value);
    }

    const auto mixdown_and_convert{[=](const auto& i) {
        return map_to_vector(
                mixdown(raytracer::dirac_histogram(
                        i.begin(), i.end(), speed_of_sound_, sample_rate, 20)),
                [=](auto i) {
                    return intensity_to_pressure(i, acoustic_impedance_) * 0.1f;
                });
    }};

    static auto count{0};
    for (const auto& i : impulses_by_depth) {
        const auto img_src_results{mixdown_and_convert(i.second)};
        snd::write(build_string("img_src_depth_", i.first, "_", count, ".wav"),
                   {img_src_results},
                   sample_rate,
                   16);
    }
    count += 1;
    return {mixdown_and_convert(impulses_by_depth.begin()->second),
            sample_rate,
            "img_src_depth"};
}
