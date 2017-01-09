#include "raytracer/reflection_processor/image_source.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/postprocess_branches.h"

#include "core/pressure_intensity.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

image_source_group_processor::image_source_group_processor(size_t max_order,
                                                           size_t items)
        : max_image_source_order_{max_order}
        , builder_{items} {}

////////////////////////////////////////////////////////////////////////////////

image_source_processor::image_source_processor(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t max_order)
        : source_{source}
        , receiver_{receiver}
        , environment_{environment}
        , voxelised_{voxelised}
        , max_order_{max_order} {}

image_source_group_processor image_source_processor::get_group_processor(
        size_t num_directions) const {
    return {max_order_, num_directions};
}

void image_source_processor::accumulate(
        const image_source_group_processor& processor) {
    for (const auto& path : processor.get_results()) {
        tree_.push(path);
    }
}

util::aligned::vector<impulse<8>> image_source_processor::get_results() const {
    //  Fetch the image source results.
    auto ret = raytracer::image_source::postprocess_branches(
            begin(tree_.get_branches()),
            end(tree_.get_branches()),
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

////////////////////////////////////////////////////////////////////////////////

make_image_source::make_image_source(size_t max_order)
        : max_order_{max_order} {}

image_source_processor make_image_source::get_processor(
        const core::compute_context& /*cc*/,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised) const {
    return {source, receiver, environment, voxelised, max_order_};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
