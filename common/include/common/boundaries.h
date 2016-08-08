#pragma once

#include "common/aligned/vector.h"
#include "common/box.h"
#include "common/scene_data.h"

#include "glm/glm.hpp"

namespace geo {
class ray;
}  // namespace geo

class boundary {
public:
    boundary() = default;
    virtual ~boundary() noexcept = default;
    boundary(boundary&&) noexcept = default;
    boundary& operator=(boundary&&) noexcept = default;
    boundary(const boundary&) = default;
    boundary& operator=(const boundary&) = default;

    virtual bool inside(const glm::vec3& v) const = 0;
    virtual box<3> get_aabb() const = 0;

    template <typename Archive>
    void serialize(Archive& archive);
};

class cuboid_boundary : public boundary {
public:
    cuboid_boundary() = default;
    cuboid_boundary(const glm::vec3& c0, const glm::vec3& c1);

    bool inside(const glm::vec3& v) const override;
    box<3> get_aabb() const override;

    template <typename Archive>
    void serialize(Archive& archive);

private:
    box<3> boundary;
};

class sphere_boundary : public boundary {
public:
    explicit sphere_boundary(const glm::vec3& c = glm::vec3(),
                             float radius = 0,
                             const aligned::vector<surface>& surfaces =
                                     aligned::vector<surface>{surface{}});
    bool inside(const glm::vec3& v) const override;
    box<3> get_aabb() const override;

private:
    glm::vec3 c;
    float radius;
    box<3> boundary;
};
