#include "raytracer/reflection_processor/visual.h"

namespace wayverb {
namespace raytracer {
namespace reflection_processor {

visual::visual(size_t items)
        : builder_{items} {}

util::aligned::vector<util::aligned::vector<reflection>> visual::get_results() {
    return std::move(builder_.get_data());
}

make_visual::make_visual(size_t items)
        : items_{items} {}

visual make_visual::operator()(
        const core::compute_context& cc,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const core::environment& environment,
        const core::voxelised_scene_data<cl_float3,
                                         core::surface<core::simulation_bands>>&
                voxelised,
        size_t num_directions) const {
    return visual{items_};
}

}  // namespace reflection_processor
}  // namespace raytracer
}  // namespace wayverb
