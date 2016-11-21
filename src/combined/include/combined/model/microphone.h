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

    microphone& operator=(microphone other);

    void set_orientation(const core::orientation& o);
    void set_shape(float shape);

    core::attenuator::microphone get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(microphone_);
    }

private:
    void swap(microphone& other) noexcept;

    core::attenuator::microphone microphone_;
};

bool operator==(const microphone& a, const microphone& b);
bool operator!=(const microphone& a, const microphone& b);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
