#include "waveguide/rectangular_mesh.h"
#include "waveguide/tetrahedral_mesh.h"

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
    auto get_mesh(const SceneData& sd) {
        return MeshType(MeshBoundary(sd), 0.1, glm::vec3(0));
    }

    template <typename MeshType>
    void locator_index_test() {
        SceneData sd(OBJ_PATH_BEDROOM);
        auto mesh = get_mesh<MeshType>(SceneData(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            ASSERT_EQ(i, mesh.compute_index(loc));
        }
    }

    template <typename MeshType>
    void test_position_index() {
        auto mesh = get_mesh<MeshType>(SceneData(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            auto pos = mesh.compute_position(loc);
            ASSERT_TRUE(loc == mesh.compute_locator(pos));
        }
    }

    template <typename MeshType>
    void test_neighbor() {
        auto mesh = get_mesh<MeshType>(SceneData(OBJ_PATH_BEDROOM));
        for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
            auto loc = mesh.compute_locator(i);
            auto pos = mesh.compute_position(loc);
            for (auto j : mesh.BaseMesh::compute_neighbors(i)) {
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
    ComputeContext compute_context;
    cl::CommandQueue queue{compute_context.context, compute_context.device};
    RectangularProgram program{RectangularProgram(compute_context.context,
                                                  compute_context.device)};
};

TEST_F(MeshTest, locator_index_tetra) {
    locator_index_test<TetrahedralMesh>();
}

TEST_F(MeshTest, locator_index_rect) {
    locator_index_test<RectangularMesh>();
}

bool operator==(const TetrahedralMesh::Locator& a,
                const TetrahedralMesh::Locator& b) {
    return std::tie(a.pos, a.mod_ind) == std::tie(b.pos, b.mod_ind);
}

TEST_F(MeshTest, position_index_tetra) {
    test_position_index<TetrahedralMesh>();
}

TEST_F(MeshTest, position_index_rect) {
    test_position_index<RectangularMesh>();
}

TEST_F(MeshTest, neighbor_tetra) {
    test_neighbor<TetrahedralMesh>();
}

TEST_F(MeshTest, neighbor_rect) {
    test_neighbor<RectangularMesh>();
}
