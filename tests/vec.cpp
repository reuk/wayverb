#include "vec.h"

//  tests
static_assert((Vec3f(1, 2, 3) + Vec3f(4, 5, 6) == Vec3f(5, 7, 9)).all(),
              "vector fail");

static_assert((Vec3f(1, 2, 3) * 3 == Vec3f(3, 6, 9)).all(), "vector fail");

static_assert(Vec3f(4, 6, 8).product() == 192, "vector fail");

static_assert(Vec3f(4, 6, 8).sum() == 18, "vector fail");
