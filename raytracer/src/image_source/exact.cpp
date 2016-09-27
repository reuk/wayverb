#include "raytracer/image_source/exact.h"

namespace raytracer {
namespace image_source {

static_assert(power(5, 0) == 1, "power");
static_assert(power(5, 1) == 5, "power");
static_assert(power(5, 5) == 3125, "power");

}  // namesapce image_source
}  // namesapce raytracer
