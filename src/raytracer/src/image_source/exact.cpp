#include "raytracer/image_source/exact.h"

namespace wayverb {
namespace raytracer {
namespace image_source {

//  Combined Wave and Ray Based Room Acoustic Simulations of Small Rooms
//  Marc Aretz, p. 71

glm::vec3 image_source_position(const glm::ivec3& order,
                                const glm::vec3& source,
                                const glm::vec3& dim) {
    return glm::vec3{order} * dim +
           glm::mix(glm::vec3{dim - source},
                    glm::vec3{source},
                    glm::equal(order % 2, glm::ivec3{0}));
}

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
