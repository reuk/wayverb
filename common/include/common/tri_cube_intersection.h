#pragma once

// see:
// http://www.realtimerendering.com/resources/GraphicsGems/gemsiii/triangleCube.c

#include "triangle_vec.h"
#include "vec_forward.h"

enum class Rel {
    idInside,
    idOutside,
};

template <typename T, typename U>
inline T lerp(U a, T b, T c) {
    return T(b + (c - b) * a);
}

int face_plane(const Vec3f& p);
int bevel_2d(const Vec3f& p);
int bevel_3d(const Vec3f& p);
int check_point(const Vec3f& p1, const Vec3f& p2, float alpha, int mask);
Rel check_line(const Vec3f& p1, const Vec3f& p2, int outcode_diff);
Rel point_triangle_intersection(const Vec3f& p, const TriangleVec3f& t);
Rel t_c_intersection(const TriangleVec3f& t);
