#pragma once

// see:
// http://www.realtimerendering.com/resources/GraphicsGems/gemsiii/triangleCube.c

#include "triangle_vec.h"

#include "glm/glm.hpp"

enum class Rel {
    idInside,
    idOutside,
};

int face_plane(const glm::vec3& p);
int bevel_2d(const glm::vec3& p);
int bevel_3d(const glm::vec3& p);
int check_point(const glm::vec3& p1,
                const glm::vec3& p2,
                float alpha,
                int mask);
Rel check_line(const glm::vec3& p1, const glm::vec3& p2, int outcode_diff);
Rel point_triangle_intersection(const glm::vec3& p, const TriangleVec3& t);
Rel t_c_intersection(const TriangleVec3& t);
