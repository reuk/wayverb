#include "iterative_tetrahedral_mesh.h"

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

TEST(mesh, locator_index) {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    IterativeTetrahedralMesh mesh(boundary, 0.1, Vec3f(0));

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        auto loc = mesh.get_locator(i);
        ASSERT_EQ(i, mesh.get_index(loc));
    }
}

bool operator==(const IterativeTetrahedralMesh::Locator& a,
                const IterativeTetrahedralMesh::Locator& b) {
    return (a.pos == b.pos).all() && a.mod_ind == b.mod_ind;
}

TEST(mesh, position_index) {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    IterativeTetrahedralMesh mesh(boundary, 0.1, Vec3f(0));

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        auto loc = mesh.get_locator(i);
        auto pos = mesh.get_position(loc);
        ASSERT_EQ(loc, mesh.get_locator(pos));
    }
}

TEST(mesh, neighbor) {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);
    IterativeTetrahedralMesh mesh(boundary, 0.1, Vec3f(0));

    auto run_test = [&mesh](auto i) {
        auto loc = mesh.get_locator(i);
        auto pos = mesh.get_position(loc);
        for (auto j : mesh.get_neighbors(i)) {
            if (j != -1) {
                auto ll = mesh.get_locator(j);
                auto pp = mesh.get_position(ll);
                ASSERT_NEAR(mesh.get_spacing(), (pos - pp).mag(), 0.0001)
                    << i << ", " << j;
            }
        }
    };

    for (auto i = 0u; i != mesh.get_nodes().size(); ++i) {
        run_test(i);
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