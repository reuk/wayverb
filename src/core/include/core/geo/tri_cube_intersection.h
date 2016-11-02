#pragma once

// see:
// http://www.realtimerendering.com/resources/GraphicsGems/gemsiii/triangleCube.c

#include "triangle_vec.h"

#include "glm/fwd.hpp"

namespace wayverb {
namespace core {
namespace geo {

enum class where { inside, outside };

int face_plane(const glm::vec3& p);
int bevel_2d(const glm::vec3& p);
int bevel_3d(const glm::vec3& p);
int check_point(const glm::vec3& p1,
                const glm::vec3& p2,
                float alpha,
                int mask);
where check_line(const glm::vec3& p1, const glm::vec3& p2, int outcode_diff);
where point_triangle_intersection(const glm::vec3& p, const triangle_vec3& t);
where t_c_intersection(const triangle_vec3& t);

}  // namespace geo
}  // namespace core
}  // namespace wayverb
