#include "waveguide/mesh.h"

#include "core/cl/common.h"
#include "core/scene_data_loader.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "utilities/progress_bar.h"

#include "gtest/gtest.h"

#include <algorithm>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#define OPT (0)
#if OPT == 0
#define THE_MODEL OBJ_PATH_TUNNEL
#elif OPT == 1
#define THE_MODEL OBJ_PATH_BEDROOM
#endif

namespace {
template <typename Vertex, typename Surface>
auto get_voxelised(const core::generic_scene_data<Vertex, Surface>& sd) {
    return make_voxelised_scene_data(sd, 5, 0.3f);
}

struct mesh_fixture : public ::testing::Test {
    using vsd =
            core::voxelised_scene_data<cl_float3,
                                       core::surface<core::simulation_bands>>;

    auto get_mesh(const vsd& voxelised) {
        const auto buffers{make_scene_buffers(cc.context, voxelised)};
        return waveguide::compute_mesh(cc, voxelised, 0.1, 340);
    }

    const core::compute_context cc;
    cl::CommandQueue queue{cc.context, cc.device};
    const vsd voxelised{get_voxelised(scene_with_extracted_surfaces(
            core::scene_data_loader{THE_MODEL}.get_scene_data()))};
};

TEST_F(mesh_fixture, locator_index) {
    const auto mesh{get_mesh(voxelised)};
    const auto lim{mesh.get_structure().get_condensed_nodes().size()};
    for (auto i{0u}; i != lim; ++i) {
        const auto loc{compute_locator(mesh.get_descriptor(), i)};
        ASSERT_EQ(i, compute_index(mesh.get_descriptor(), loc));
    }
}

TEST_F(mesh_fixture, position_index) {
    const auto mesh{get_mesh(voxelised)};
    const auto lim{mesh.get_structure().get_condensed_nodes().size()};
    for (auto i{0u}; i != lim; ++i) {
        const auto loc{compute_locator(mesh.get_descriptor(), i)};
        const auto pos{compute_position(mesh.get_descriptor(), loc)};
        ASSERT_TRUE(loc == compute_locator(mesh.get_descriptor(), pos));
    }
}

}  // namespace
