#include "waveguide/mesh.h"

#include "core/spatial_division/voxelised_scene_data.h"
#include "core/scene_data_loader.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

namespace {

template <typename Vertex, typename Surface>
auto get_voxelised(const generic_scene_data<Vertex, Surface>& scene) {
    return make_voxelised_scene_data(scene, 5, 0.1f);
}

TEST(mesh_setup, setup) {
    const auto boundary{get_voxelised(scene_with_extracted_surfaces(
            scene_data_loader{OBJ_PATH_BEDROOM}.get_scene_data()))};
    const auto m{
            waveguide::compute_mesh(compute_context{}, boundary, 0.1, 340)};
}

}  // namespace
