#include "waveguide/mesh_boundary.h"

#include "common/almost_equal.h"
#include "common/geo/overlaps_2d.h"
#include "common/geo/rect.h"
#include "common/scene_data.h"
#include "common/spatial_division.h"
#include "common/stl_wrappers.h"

namespace waveguide {

glm::ivec2 mesh_boundary::hash_point(const glm::vec2& v) const {
    return (v - triangle_references.get_aabb().get_min()) / cell_size;
}

mesh_boundary::mesh_boundary(const copyable_scene_data& sd)
        : scene_data(sd)
        , triangle_references(ndim_tree<2>(
                  8,
                  [&](auto item, const auto& aabb) {
                      return geo::overlaps(
                              aabb,
                              geo::get_triangle_vec2(sd.get_triangles()[item],
                                                     sd.get_vertices()));
                  },
                  sd.get_triangle_indices(),
                  padded(geo::rect(sd.get_aabb().get_min(),
                                   sd.get_aabb().get_max()),
                         glm::vec2(0.1))))
        , cell_size(voxel_dimensions(triangle_references)) {}

const aligned::vector<size_t>& mesh_boundary::get_references(
        const glm::ivec2& i) const {
    if ((0 <= i.x && i.x < triangle_references.get_side()) &&
        (0 <= i.y && i.y < triangle_references.get_side())) {
        return triangle_references.get_voxel(i);
    }
    return empty;
}

bool mesh_boundary::inside(const glm::vec3& v) const {
    //  cast ray through point along Z axis and check for intersections
    //  with each of the referenced triangles then count number of intersections
    //  on one side of the point
    //  if intersection number is even, point is outside, else it's inside
    const auto references = get_references(hash_point(v));
    const geo::ray ray{v, glm::vec3(0, 0, 1)};
    auto distances = aligned::vector<float>();
    return proc::count_if(references,
                          [&](const auto& i) {
                              const auto intersection = triangle_intersection(
                                      scene_data.get_triangles()[i],
                                      scene_data.get_vertices(),
                                      ray);
                              if (intersection) {
                                  const auto already_in =
                                          proc::find_if(
                                                  distances,
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

geo::box mesh_boundary::get_aabb() const { return scene_data.get_aabb(); }

const copyable_scene_data& mesh_boundary::get_scene_data() const {
    return scene_data;
}

const aligned::vector<size_t> mesh_boundary::empty{};

}  // namespace waveguide
