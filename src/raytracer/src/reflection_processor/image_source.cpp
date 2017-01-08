#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/postprocess_branches.h"

#include "core/pressure_intensity.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

image_source::image_source(
        size_t max_order,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t items)
        : source_{source}
        , receiver_{receiver}
        , environment_{environment}
        , voxelised_{voxelised}
        , max_image_source_order_{max_order}
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
            source_,
            receiver_,
            voxelised_,
            false);

    //  Add the line-of-sight contribution, which isn't directly detected by
    //  the image-source machinery.
    using namespace image_source;
    if (const auto direct = get_direct(source_, receiver_, voxelised_)) {
        ret.emplace_back(*direct);
    }

    //  Correct for distance travelled.
    for (auto& imp : ret) {
        imp.volume *= core::pressure_for_distance(
                imp.distance, environment_.acoustic_impedance);
    }

    return ret;
}

make_image_source::make_image_source(size_t max_order)
        : max_order_{max_order} {}

image_source make_image_source::operator()(
        const core::compute_context& /*cc*/,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t num_directions) const {
    return {max_order_,
            source,
            receiver,
            environment,
            voxelised,
            num_directions};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
