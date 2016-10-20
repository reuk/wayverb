#include "waveguide/mesh.h"

#include "common/cl/common.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/voxelised_scene_data.h"

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
auto get_voxelised(const generic_scene_data<Vertex, Surface>& sd) {
    return make_voxelised_scene_data(sd, 5, 0.3f);
}

struct mesh_fixture : public ::testing::Test {
    using vsd = voxelised_scene_data<cl_float3, surface<simulation_channels>>;

    auto get_mesh(const vsd& voxelised) {
        const auto buffers{make_scene_buffers(cc.context, voxelised)};
        return waveguide::compute_mesh(cc, voxelised, 0.1, 340);
    }

    const compute_context cc;
    cl::CommandQueue queue{cc.context, cc.device};
    const vsd voxelised{get_voxelised(scene_with_extracted_surfaces(
            scene_data_loader{THE_MODEL}.get_scene_data()))};
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
        const auto loc{compute_locator(mesh    .get_descriptor(), i)};
        const auto pos{compute_position(mesh   .get_descriptor(), loc)};
        ASSERT_TRUE(loc == compute_locator(mesh.get_descriptor(), pos));
    }
}

/*
TEST_F(mesh_fixture, neighbor) {
    const auto mesh{get_mesh(voxelised)};
    const auto lim{mesh.nodes.size()};
    for (auto i{0u}; i != lim; ++i) {
        const auto loc{compute_locator(mesh.descriptor, i)};
        const auto pos{compute_position(mesh.descriptor, loc)};
        for (const auto j : compute_neighbors(mesh.descriptor, i)) {
            if (j != -1) {
                const auto ll{compute_locator(mesh.descriptor, j)};
                const auto pp{compute_position(mesh.descriptor, ll)};
                ASSERT_NEAR(
                        mesh.descriptor.spacing, glm::distance(pos, pp), 0.0001)
                        << i << ", " << j;
            }
        }
    }

    const auto ports_contains{[](auto i, auto j) {
        for (auto x : i.ports) {
            if (x == j)
                return true;
        }
        return false;
    }};

    for (auto i{0u}; i != lim; ++i) {
        const auto& root{mesh.nodes[i]};
        for (auto j{0u}; j != 6; ++j) {
            auto ind{root.ports[j]};
            if (ind != -1)
                ASSERT_TRUE(ports_contains(mesh.nodes[ind], i));
        }
    }
}
*/

/*
TEST_F(mesh_fixture, inside) {
    const auto mesh{get_mesh(voxelised)};

#if 0
    const auto directions{raytracer::get_random_directions(32)};
    std::cout << "constant float3 random_directions[] = {\n";
    for (const auto& i : directions) {
        std::cout << "    (float3)(" << i.x << ", " << i.y << ", " << i.z
                  << "),\n";
    }
    std::cout << "};\n";
#endif

    const auto cpu_inside{map_to_vector(mesh.nodes, [&](const auto& i) {
        return inside(voxelised, to_vec3(i.position));
    })};

    auto pb{progress_bar{std::cout, mesh.nodes.size()}};
    auto same{0u};
    for (auto i{0u}; i != mesh.nodes.size(); ++i, pb += 1) {
        if (mesh.nodes[i].inside == cpu_inside[i]) {
            same += 1;
        } else {
            // std::cerr << "mismatch at index " << i << '\n';
            // std::cerr << "    gpu: " << static_cast<bool>(nodes[i].inside)
            //          << ", cpu: " << cpu_inside[i] << '\n';
        }
    }

    const auto percentage_similar{(same * 100.0) / mesh.nodes.size()};

    if (same != mesh.nodes.size()) {
        std::cerr << same << " nodes are the same out of " << mesh.nodes.size()
                  << '\n';
        std::cerr << percentage_similar << "% match\n";
    }

    ASSERT_TRUE(95 <= percentage_similar);
}
*/

}  // namespace
