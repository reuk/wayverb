#pragma once

#include "glm/glm.hpp"

namespace wayverb {
namespace core {


/// Re-writing at the last minute so that this uses a right-handed coordinate 
/// system, similar to OpenGL.
/// Positive-y is up, positive-x is right, and *negative-z* is forwards.
/// Yeah ugh idk.

/// Invariant: pointing_ is a unit vector.
class orientation final {
public:
    explicit orientation(const glm::vec3& pointing = {0, 0, -1},
                         const glm::vec3& up = {0, 1, 0});

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    glm::vec3 get_up() const;
    void set_up(const glm::vec3& u);

    glm::mat4 get_matrix() const;

    template <typename Archive>
    void serialize(Archive&);

private:
    glm::vec3 pointing_;
    glm::vec3 up_;
};

bool operator==(const orientation& a, const orientation& b);
bool operator!=(const orientation& a, const orientation& b);

orientation combine(const orientation& a, const orientation& b);

//  Given an object, oriented relative to world-space, and a vector direction
//  relative to world-space, find the vector direction relative to the 
//  oriented object.
glm::vec3 transform(const orientation& orientation, const glm::vec3& vec);

}  // namespace core
}  // namespace wayverb
