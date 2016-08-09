#include "common/boundaries.h"

#include "common/aligned/vector.h"
#include "common/almost_equal.h"
#include "common/conversions.h"
#include "common/geo/geometric.h"
#include "common/geo/tri_cube_intersection.h"
#include "common/scene_data.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/triangle.h"

#include "glog/logging.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

cuboid_boundary::cuboid_boundary(const glm::vec3& c0, const glm::vec3& c1)
        : boundary(c0, c1) {}

bool cuboid_boundary::inside(const glm::vec3& v) const {
    return util::inside(boundary, v);
}

geo::box cuboid_boundary::get_aabb() const { return boundary; }

sphere_boundary::sphere_boundary(const glm::vec3& c,
                                 float radius,
                                 const aligned::vector<surface>& surfaces)
        : c(c)
        , radius(radius)
        , boundary(glm::vec3(-radius), glm::vec3(radius)) {}

bool sphere_boundary::inside(const glm::vec3& v) const {
    return glm::distance(v, c) < radius;
}

geo::box sphere_boundary::get_aabb() const { return boundary; }
