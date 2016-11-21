#pragma once

#include "combined/model/capsule.h"
#include "combined/model/hover.h"
#include "combined/model/min_size_vector.h"

#include "cereal/types/base_class.hpp"

namespace wayverb {
namespace combined {
namespace model {

class receiver final
        : public owning_member<receiver, vector<capsule>, hover_state> {
public:
    explicit receiver(std::string name = "new receiver",
                      glm::vec3 position = glm::vec3{0},
                      core::orientation orientation = core::orientation{});

    receiver& operator=(receiver other);

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& p);
    glm::vec3 get_position() const;

    void set_orientation(const core::orientation& orientation);
    core::orientation get_orientation() const;

    const auto& capsules() const { return get<0>(); }

    using hover_state_t = class hover_state;
    const auto& hover_state() const { return get<1>(); }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(capsules()->item, name_, position_, orientation_);
    }

private:
    void swap(receiver& other) noexcept;

    std::string name_;
    glm::vec3 position_;
    core::orientation orientation_;
};

bool operator==(const receiver& a, const receiver& b);
bool operator!=(const receiver& a, const receiver& b);

////////////////////////////////////////////////////////////////////////////////

using receivers = min_size_vector<receiver, 1>;

}  // namespace model
}  // namespace combined
}  // namespace wayverb
