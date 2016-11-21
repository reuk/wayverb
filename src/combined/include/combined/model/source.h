#pragma once

#include "combined/model/hover.h"
#include "combined/model/min_size_vector.h"

#include "core/geo/box.h"

#include "cereal/types/base_class.hpp"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class source final : public owning_member<source, hover_state> {
public:
    explicit source(std::string name = "new source",
                    glm::vec3 position = glm::vec3{0});

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const;

    using hover_state_t = class hover_state;
    const auto& hover_state() const { return get<0>(); }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(name_, position_);
    }

private:
    std::string name_;
    glm::vec3 position_;
};

bool operator==(const source& a, const source& b);
bool operator!=(const source& a, const source& b);

////////////////////////////////////////////////////////////////////////////////

using sources = min_size_vector<source, 1>;

}  // namespace model
}  // namespace combined
}  // namespace wayverb
