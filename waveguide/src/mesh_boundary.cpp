#include "waveguide/mesh_boundary.h"

#include "common/almost_equal.h"
#include "common/box.h"
#include "common/overlaps_2d.h"
#include "common/scene_data.h"
#include "common/spatial_division.h"
#include "common/stl_wrappers.h"

glm::ivec2 mesh_boundary::hash_point(const glm::vec3& v) const {
    return glm::vec2(v - boundary.get_c0()) / cell_size;
}

mesh_boundary::mesh_boundary(const copyable_scene_data& sd)
        : triangles(sd.get_triangles())
        , vertices(sd.get_converted_vertices())
        , surfaces(sd.get_surfaces())
        , boundary(sd.get_aabb())
        , triangle_references(ndim_tree<2>(
                  10,
                  [&](auto item, const auto& aabb) {
                      return ::overlaps(
                              aabb,
                              get_triangle_vec2(triangles[item], vertices));
                  },
                  sd.get_triangle_indices(),
                  box<2>(sd.get_aabb().get_c0(), sd.get_aabb().get_c1())))
        , cell_size(::dimensions(triangle_references.get_voxel_aabb())) {}

const aligned::vector<size_t>& mesh_boundary::get_references(
        const glm::ivec2& i) const {
    return get_references(i.x, i.y);
}

const aligned::vector<size_t>& mesh_boundary::get_references(int x,
                                                             int y) const {
    if (0 <= x && x < triangle_references.get_side() && 0 <= y &&
        y < triangle_references.get_side()) {
        return triangle_references.get_data()[x][y].get_items();
    }
    return empty;
}

bool mesh_boundary::inside(const glm::vec3& v) const {
    //  cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles then count number of intersections
    //  on one side of the point
    //  if intersection number is even, point is outside, else it's inside
    const auto references = get_references(hash_point(v));
    geo::ray ray{v, glm::vec3(0, 0, 1)};
    auto distances = aligned::vector<float>();
    return count_if(references.begin(),
                    references.end(),
                    [this, &ray, &distances](const auto& i) {
                        auto intersection = triangle_intersection(
                                triangles[i], vertices, ray);
                        if (intersection) {
                            auto already_in =
                                    proc::find_if(distances,
                                                  [&intersection](auto i) {
                                                      return almost_equal(
                                                              i,
                                                              *intersection,
                                                              size_t{10});
                                                  }) != distances.end();
                            distances.push_back(*intersection);
                            if (already_in) {
                                return false;
                            }
                        }
                        return static_cast<bool>(intersection);
                    }) %
           2;
}

box<3> mesh_boundary::get_aabb() const {
    return boundary; }

aligned::vector<size_t> mesh_boundary::get_triangle_indices() const {
    aligned::vector<size_t> ret(triangles.size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<triangle>& mesh_boundary::get_triangles() const {
    return triangles;
}

const aligned::vector<glm::vec3>& mesh_boundary::get_vertices() const {
    return vertices;
}

const aligned::vector<surface>& mesh_boundary::get_surfaces() const {
    return surfaces;
}

const aligned::vector<size_t> mesh_boundary::empty{};
