#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/postprocess_branches.h"

#include "core/pressure_intensity.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

image_source::image_source(
        size_t max_order,
        bool flip_phase,
        const core::model::parameters& params,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t items)
        : params_{params}
        , voxelised_{voxelised}
        , max_image_source_order_{max_order}
        , flip_phase_{flip_phase}
        , builder_{items} {}

util::aligned::vector<impulse<8>> image_source::get_results() {
    raytracer::image_source::tree tree{};
    for (const auto& path : builder_.get_data()) {
        tree.push(path);
    }

    //  Fetch the image source results.
    auto ret = raytracer::image_source::postprocess_branches(
            begin(tree.get_branches()),
            end(tree.get_branches()),
            params_.source,
            params_.receiver,
            voxelised_,
            flip_phase_);

    //  Add the line-of-sight contribution, which isn't directly detected by
    //  the image-source machinery.
    using namespace image_source;
    if (const auto direct =
                get_direct(params_.source, params_.receiver, voxelised_)) {
        ret.emplace_back(*direct);
    }

    //  Correct for distance travelled.
    for (auto& imp : ret) {
        imp.volume *= core::pressure_for_distance(imp.distance,
                                                  params_.acoustic_impedance);
    }

    return ret;
}

make_image_source::make_image_source(size_t max_order, bool flip_phase)
        : max_order_{max_order}
        , flip_phase_{flip_phase} {}

image_source make_image_source::operator()(
        const core::compute_context& cc,
        const core::model::parameters& params,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t num_directions) const {
    return {max_order_, flip_phase_, params, voxelised, num_directions};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
