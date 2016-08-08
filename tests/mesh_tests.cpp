#include "waveguide/mesh.h"
#include "waveguide/mesh_boundary.h"

#include "common/cl_common.h"
#include "common/scene_data.h"

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
    template <typename MeshType>
    auto get_mesh(const copyable_scene_data& sd) {
        return MeshType(mesh_boundary(sd), 0.1, glm::vec3(0));
    }

    template <typename MeshType>
    void locator_index_test() {
        scene_data sd(OBJ_PATH_BEDROOM);
        auto mesh = get_mesh<MeshType>(scene_data(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            ASSERT_EQ(i, mesh.compute_index(loc));
        }
    }

    template <typename MeshType>
    void test_position_index() {
        auto mesh = get_mesh<MeshType>(scene_data(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            auto pos = mesh.compute_position(loc);
            ASSERT_TRUE(loc == mesh.compute_locator(pos));
        }
    }

    template <typename MeshType>
    void test_neighbor() {
        auto mesh = get_mesh<MeshType>(scene_data(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            auto pos = mesh.compute_position(loc);
            for (auto j : mesh.compute_neighbors(i)) {
                if (j != -1) {
                    auto ll = mesh.compute_locator(j);
                    auto pp = mesh.compute_position(ll);
                    ASSERT_NEAR(
                            mesh.get_spacing(), glm::distance(pos, pp), 0.0001)
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

        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            const auto& root = mesh.get_nodes()[i];
            for (auto j = 0u; j != 4; ++j) {
                auto ind = root.ports[j];
                if (ind != -1)
                    ASSERT_TRUE(ports_contains(mesh.get_nodes()[ind], i));
            }
        }
    }

private:
    compute_context cc;
    cl::CommandQueue queue{cc.get_context(), cc.get_device()};
    waveguide::program program{cc.get_context(), cc.get_device()};
};

TEST_F(MeshTest, locator_index_rect) { locator_index_test<waveguide::mesh>(); }

TEST_F(MeshTest, position_index_rect) {
    test_position_index<waveguide::mesh>();
}

TEST_F(MeshTest, neighbor_rect) { test_neighbor<waveguide::mesh>(); }
