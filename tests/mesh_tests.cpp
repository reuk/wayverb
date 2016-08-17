#include "waveguide/mesh/model.h"

#include "raytracer/reflector.h"

#include "common/cl_common.h"
#include "common/progress_bar.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/timed_scope.h"

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
auto get_voxelised(const copyable_scene_data& sd) {
    return voxelised_scene_data{
            sd, 5, util::padded(sd.get_aabb(), glm::vec3{0.3})};
}

struct mesh_fixture : public ::testing::Test {
    auto get_mesh(const voxelised_scene_data& voxelised) {
        return waveguide::mesh::compute_fat_nodes(
                cc.get_context(), cc.get_device(), voxelised, 0.1);
    }

    const compute_context cc;
    const waveguide::program program{cc.get_context(), cc.get_device()};
    cl::CommandQueue queue{cc.get_context(), cc.get_device()};
    const voxelised_scene_data voxelised{
            get_voxelised(scene_data{THE_MODEL})};
};

TEST_F(mesh_fixture, locator_index) {
    aligned::vector<node> nodes;
    waveguide::mesh::descriptor desc;
    std::tie(nodes, desc) = get_mesh(voxelised);
    const auto lim = nodes.size();
    for (auto i = 0u; i != lim; ++i) {
        const auto loc = compute_locator(desc, i);
        ASSERT_EQ(i, compute_index(desc, loc));
    }
}

TEST_F(mesh_fixture, position_index) {
    aligned::vector<node> nodes;
    waveguide::mesh::descriptor desc;
    std::tie(nodes, desc) = get_mesh(voxelised);
    const auto lim = nodes.size();
    for (auto i = 0u; i != lim; ++i) {
        const auto loc = compute_locator(desc, i);
        const auto pos = compute_position(desc, loc);
        ASSERT_TRUE(loc == compute_locator(desc, pos));
    }
}

TEST_F(mesh_fixture, neighbor) {
    aligned::vector<node> nodes;
    waveguide::mesh::descriptor desc;
    std::tie(nodes, desc) = get_mesh(voxelised);
    const auto lim = nodes.size();
    for (auto i = 0u; i != lim; ++i) {
        const auto loc = compute_locator(desc, i);
        const auto pos = compute_position(desc, loc);
        for (const auto j : compute_neighbors(desc, i)) {
            if (j != -1) {
                const auto ll = compute_locator(desc, j);
                const auto pp = compute_position(desc, ll);
                ASSERT_NEAR(desc.spacing, glm::distance(pos, pp), 0.0001)
                        << i << ", " << j;
            }
        }
    }

    auto ports_contains = [](auto i, auto j) {
        for (auto x : i.ports) {
            if (x == j)
                return true;
        }
        return false;
    };

    for (auto i = 0u; i != lim; ++i) {
        const auto& root = nodes[i];
        for (auto j = 0u; j != 6; ++j) {
            auto ind = root.ports[j];
            if (ind != -1)
                ASSERT_TRUE(ports_contains(nodes[ind], i));
        }
    }
}

TEST_F(mesh_fixture, inside) {
    aligned::vector<node> nodes;
    waveguide::mesh::descriptor desc;
    std::tie(nodes, desc) = get_mesh(voxelised);

#if 0
    const auto directions{raytracer::get_random_directions(32)};
    std::cout << "constant float3 random_directions[] = {\n";
    for (const auto& i : directions) {
        std::cout << "    (float3)(" << i.x << ", " << i.y << ", " << i.z
                  << "),\n";
    }
    std::cout << "};\n";
#endif

    const auto cpu_inside{map_to_vector(nodes, [&](const auto& i) {
        return inside(voxelised, to_vec3(i.position));
    })};

    auto pb{progress_bar{std::cout, nodes.size()}};
    auto same{0u};
    for (auto i{0u}; i != nodes.size(); ++i, pb += 1) {
        if (nodes[i].inside == cpu_inside[i]) {
            same += 1;
        } else {
            //std::cerr << "mismatch at index " << i << '\n';
            //std::cerr << "    gpu: " << static_cast<bool>(nodes[i].inside)
            //          << ", cpu: " << cpu_inside[i] << '\n';
        }
    }

    const auto percentage_similar{(same * 100.0) / nodes.size()};

    if (same != nodes.size()) {
        std::cerr << same << " nodes are the same out of " << nodes.size()
                  << '\n';
        std::cerr << percentage_similar << "% match\n";
    }

    ASSERT_TRUE(95 <= percentage_similar);
}

}  // namespace
