#pragma once

#include "combined/model/capsule.h"
#include "combined/model/vector.h"

#include "core/geo/box.h"

namespace wayverb {
namespace combined {
namespace model {

class receiver final : public member<receiver, vector<capsule>> {
public:
    receiver(core::geo::box bounds);

    receiver(const receiver&) = delete;
    receiver(receiver&&) noexcept = delete;

    receiver& operator=(const receiver&) = delete;
    receiver& operator=(receiver&&) noexcept = delete;

    void set_name(std::string name);
    void set_position(const glm::vec3& position);
    void set_orientation(float azimuth, float elevation);

    const capsule& get_capsule(size_t index) const;
    capsule& get_capsule(size_t index);
    void add_capsule(size_t index);
    void remove_capsule(size_t index);

private:
    core::geo::box bounds_;

    std::string name_ = "new receiver";
    glm::vec3 position_;
    core::orientable orientation_;
    vector<capsule> capsules_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
