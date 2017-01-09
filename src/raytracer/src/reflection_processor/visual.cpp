#include "raytracer/reflection_processor/visual.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

visual_group_processor::visual_group_processor(size_t items)
        : builder_{items} {}

////////////////////////////////////////////////////////////////////////////////

visual_processor::visual_processor(size_t items)
        : items_{items} {}

visual_group_processor visual_processor::get_group_processor(
        size_t /*num_directions*/) const {
    return visual_group_processor{items_};
}

void visual_processor::accumulate(const visual_group_processor& processor) {
    if (results_.empty()) {
        results_ = processor.get_results();
    }
}

util::aligned::vector<util::aligned::vector<reflection>>
visual_processor::get_results() {
    return results_;
}

////////////////////////////////////////////////////////////////////////////////

make_visual::make_visual(size_t items)
        : items_{items} {}

visual_processor make_visual::get_processor(
        const core::compute_context& /*cc*/,
        const glm::vec3& /*source*/,
        const glm::vec3& /*receiver*/,
        const core::environment& /*environment*/,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
        /*voxelised*/) const {
    return visual_processor{items_};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
