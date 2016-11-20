#pragma once

#include "combined/model/capsule.h"
#include "combined/model/hover.h"
#include "combined/model/vector.h"

#include "cereal/types/base_class.hpp"

namespace wayverb {
namespace combined {
namespace model {

class receiver final : public owning_member<receiver,
                                            vector<capsule, 1>,
                                            hover_state> {
public:
    explicit receiver(std::string name = "new receiver",
                      glm::vec3 position = glm::vec3{0},
                      core::orientation orientation = core::orientation{});

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& p);
    glm::vec3 get_position() const;

    void set_orientation(const core::orientation &orientation);
    core::orientation get_orientation() const;

    auto& capsules() { return get<vector<capsule, 1>>(); }
    const auto& capsules() const { return get<vector<capsule, 1>>(); }

    using hover_state_t = class hover_state;
    auto& hover_state() { return get<hover_state_t>(); }
    const auto& hover_state() const { return get<hover_state_t>(); }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(capsules(), name_, position_, orientation_);
    }

private:
    std::string name_ = "new receiver";
    glm::vec3 position_{0};
    core::orientation orientation_;
};

////////////////////////////////////////////////////////////////////////////////

using receivers = vector<receiver, 1>;

}  // namespace model
}  // namespace combined
}  // namespace wayverb
