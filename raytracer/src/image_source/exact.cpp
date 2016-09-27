#include "raytracer/image_source/exact.h"

namespace raytracer {
namespace image_source {

static_assert(power(5, 0) == 1, "power");
static_assert(power(5, 1) == 5, "power");
static_assert(power(5, 5) == 3125, "power");

void find_impulses(const geo::box& box,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   double absorption) {
    aligned::vector<double> surfaces{absorption};
    //  Create callback.
    callback_accumulator<fast_pressure_calculator<double>> callback{
            receiver, surfaces, false};

    const auto reflections{compute_optimum_reflection_number(absorption)};

    //  Traverse the image sources, summing contributions.
    traverse_images(box,
                    source,
                    reflections,
                    [&](const auto& img, auto begin, auto end) {
                        callback(img, begin, end);
                    });

    //  TODO Do something with the output.
    const auto output{callback.get_output()};
}

}  // namesapce image_source
}  // namesapce raytracer
