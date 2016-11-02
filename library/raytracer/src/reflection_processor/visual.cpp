#include "raytracer/reflection_processor/visual.h"

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
        const compute_context& cc,
        const model::parameters& params,
        const voxelised_scene_data<cl_float3, surface<simulation_bands>>&
                voxelised,
        size_t num_directions) const {
    return visual{items_};
}

}  // namespace reflection_processor
}  // namespace raytracer
