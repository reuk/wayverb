#pragma once

#include "glm/glm.hpp"

namespace wayverb {
namespace core {

/// Invariant: pointing_ is a unit vector.
class orientable final {
public:
    orientable() = default;
    explicit orientable(const glm::vec3& pointing);

    glm::vec3 get_pointing() const;
    void set_pointing(const glm::vec3& u);

    glm::mat4 get_matrix() const;

    template <typename Archive>
    void serialize(Archive&);

private:
    glm::vec3 pointing_{0, 0, 1};
};

}  // namespace core
}  // namespace wayverb
