#pragma once

#include "combined/model/hrtf.h"
#include "combined/model/microphone.h"

namespace wayverb {
namespace combined {
namespace model {

class capsule final : public member<capsule> {
public:
    capsule();

    capsule(const capsule&) = delete;
    capsule(capsule&&) noexcept = delete;

    capsule& operator=(const capsule&) = delete;
    capsule& operator=(capsule&&) noexcept = delete;

    enum class mode { microphone, hrtf };

    void set_name(std::string name);
    void set_mode(mode mode);

    microphone microphone;
    hrtf hrtf;

private:
    std::string name_ = "new capsule";
    mode mode_ = mode::microphone;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
