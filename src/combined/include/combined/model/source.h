#pragma once

#include "combined/model/member.h"

#include "core/geo/box.h"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class source final : public member<source> {
public:
    source(core::geo::box bounds);

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const;

private:
    core::geo::box bounds_;

    std::string name_;
    glm::vec3 position_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
