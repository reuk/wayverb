#include "box/raytracer.h"

#include "raytracer/raytracer.h"

#include "common/dsp_vector_ops.h"

#include "audio_file/audio_file.h"

raytracer::results run_raytracer(const geo::box& box,
                                 const surface& surface,
                                 const model::parameters& params,
                                 size_t image_source_order) {
    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, surface), 2, 0.1f)};

    const compute_context cc{};

    const auto directions{get_random_directions(1 << 16)};

    auto results{raytracer::run(begin(directions),
                                end(directions),
                                cc,
                                voxelised,
                                params,
                                0.2f,
                                image_source_order,
                                0,
                                true,
                                [](auto i, auto steps) {})};

    if (!results) {
        throw std::runtime_error{"raytracer failed to generate results"};
    }

    return std::move(*results);
}
