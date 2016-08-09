#include "common/geo/rect.h"
#include "common/serialize/boundaries.h"
#include "common/spatial_division.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

void print_nodes(const ndim_tree<3>& o, int level = 0) {
    if (o.get_nodes().empty()) {
        std::cout << "** octree node **" << std::endl;
        for (const auto& i : o.get_items()) {
            std::cout << i << std::endl;
        }
    } else {
        for (const auto& i : o.get_nodes()) {
            print_nodes(i, level + 1);
        }
    }
}

TEST(ndim_tree, constructor) {
    const scene_data scene_data(OBJ_PATH);
    const auto octree = octree_from_scene_data(scene_data, 4, 0.1);
    print_nodes(octree);
}

/*
template <typename T>
bool all_nodes_have_items(const T& t) {
    if (!t.has_nodes()) {
        return !t.get_items().empty();
    }

    return proc::all_of(t.get_nodes(),
                        [](const auto& t) { return all_nodes_have_items(t); });
}

TEST(ndim_tree, quad) {
    const scene_data scene_data(OBJ_PATH);

    {
        const auto quadtree = ndim_tree<2>(
                4,
                [&](auto item, const auto& aabb) {
                    return geo::overlaps(
                            aabb,
                            geo::get_triangle_vec2(
                                    scene_data.get_triangles()[item],
                                    scene_data.get_vertices()));
                },
                scene_data.get_triangle_indices(),
                geo::rect(scene_data.get_aabb().get_min(),
                          scene_data.get_aabb().get_max()));

        ASSERT_TRUE(all_nodes_have_items(quadtree));
    }
}
*/
