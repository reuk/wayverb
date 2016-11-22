#pragma once

#include "combined/model/member.h"

#include "core/attenuator/microphone.h"
#include "core/serialize/attenuators.h"

namespace wayverb {
namespace combined {
namespace model {

class microphone final : public basic_member<microphone> {
public:
    microphone() = default;
    microphone(const core::orientation& o, float shape);

    void set_orientation(const core::orientation& o);
    void set_shape(float shape);

    core::attenuator::microphone get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(microphone_);
    }

    NOTIFYING_COPY_ASSIGN_DECLARATION(microphone)
private:
    inline void swap(microphone& other) noexcept {
        using std::swap;
        swap(microphone_, other.microphone_);
    };

    core::attenuator::microphone microphone_;
};

bool operator==(const microphone& a, const microphone& b);
bool operator!=(const microphone& a, const microphone& b);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
