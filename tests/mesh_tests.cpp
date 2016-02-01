#include "iterative_tetrahedral_mesh.h"
#include "rectangular_mesh.h"

#include "gtest/gtest.h"

#include <algorithm>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef MAT_PATH_TUNNEL
#define MAT_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#ifndef MAT_PATH_BEDROOM
#define MAT_PATH_BEDROOM ""
#endif

template <typename MeshType>
void locator_index_test() {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    MeshType mesh(boundary, 0.1, Vec3f(0));

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        auto loc = mesh.compute_locator(i);
        ASSERT_EQ(i, mesh.compute_index(loc));
    }
}

TEST(tetra, locator_index) {
    locator_index_test<IterativeTetrahedralMesh>();
}

TEST(rect, locator_index) {
    locator_index_test<RectangularMesh>();
}

bool operator==(const IterativeTetrahedralMesh::Locator& a,
                const IterativeTetrahedralMesh::Locator& b) {
    return (a.pos == b.pos).all() && a.mod_ind == b.mod_ind;
}

bool test_equal(const Vec3i& a, const Vec3i& b) {
    return (a == b).all();
}

bool test_equal(const IterativeTetrahedralMesh::Locator& a,
                const IterativeTetrahedralMesh::Locator& b) {
    return test_equal(a.pos, b.pos) && a.mod_ind == b.mod_ind;
}

template <typename MeshType>
void test_position_index() {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    MeshType mesh(boundary, 0.1, Vec3f(0));

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        auto loc = mesh.compute_locator(i);
        auto pos = mesh.compute_position(loc);
        ASSERT_TRUE(test_equal(loc, mesh.compute_locator(pos)));
    }
}

TEST(tetra, position_index) {
    test_position_index<IterativeTetrahedralMesh>();
}

TEST(rect, position_index) {
    test_position_index<RectangularMesh>();
}

template <typename MeshType>
void test_neighbor() {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    MeshType mesh(boundary, 0.1, Vec3f(0));

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        auto loc = mesh.compute_locator(i);
        auto pos = mesh.compute_position(loc);
        for (auto j : mesh.compute_neighbors(i)) {
            if (j != -1) {
                auto ll = mesh.compute_locator(j);
                auto pp = mesh.compute_position(ll);
                ASSERT_NEAR(mesh.get_spacing(), (pos - pp).mag(), 0.0001)
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

TEST(tetra, neighbor) {
    test_neighbor<IterativeTetrahedralMesh>();
}

TEST(rect, neighbor) {
    test_neighbor<RectangularMesh>();
}
