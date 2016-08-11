#include "waveguide/mesh/model.h"

#include "common/cl_common.h"
#include "common/voxelised_scene_data.h"

#include "gtest/gtest.h"

#include <algorithm>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

class MeshTest : public ::testing::Test {
public:
    auto get_mesh(const copyable_scene_data& sd) {
        const voxelised_scene_data voxelised(sd, 5, sd.get_aabb());
        return waveguide::mesh::compute_fat_nodes(
                cc.get_context(), cc.get_device(), voxelised, 0.1);
    }

    void locator_index_test() {
        aligned::vector<waveguide::mesh::setup::node> nodes;
        waveguide::mesh::descriptor desc;
        std::tie(nodes, desc) = get_mesh(scene_data(OBJ_PATH_BEDROOM));
        const auto lim = nodes.size();
        for (auto i = 0u; i != lim; ++i) {
            const auto loc = compute_locator(desc, i);
            ASSERT_EQ(i, compute_index(desc, loc));
        }
    }

    void test_position_index() {
        aligned::vector<waveguide::mesh::setup::node> nodes;
        waveguide::mesh::descriptor desc;
        std::tie(nodes, desc) = get_mesh(scene_data(OBJ_PATH_BEDROOM));
        const auto lim = nodes.size();
        for (auto i = 0u; i != lim; ++i) {
            const auto loc = compute_locator(desc, i);
            const auto pos = compute_position(desc, loc);
            ASSERT_TRUE(loc == compute_locator(desc, pos));
        }
    }

    void test_neighbor() {
        aligned::vector<waveguide::mesh::setup::node> nodes;
        waveguide::mesh::descriptor desc;
        std::tie(nodes, desc) = get_mesh(scene_data(OBJ_PATH_BEDROOM));
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

private:
    compute_context cc;
    cl::CommandQueue queue{cc.get_context(), cc.get_device()};
    waveguide::program program{cc.get_context(), cc.get_device()};
};

TEST_F(MeshTest, locator_index_rect) { locator_index_test(); }

TEST_F(MeshTest, position_index_rect) { test_position_index(); }

TEST_F(MeshTest, neighbor_rect) { test_neighbor(); }
