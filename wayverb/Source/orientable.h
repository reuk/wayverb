#pragma once

#include "glm/glm.hpp"

/// Invariant: pointing_ is a unit vector.
class orientable final {
public:
    orientable() = default;
    orientable(const glm::vec3& pointing);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    glm::mat4 get_matrix() const;

private:
    glm::vec3 pointing_{0, 0, 1};
};
