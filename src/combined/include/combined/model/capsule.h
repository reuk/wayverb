#pragma once

#include "combined/model/hrtf.h"
#include "combined/model/microphone.h"

namespace wayverb {
namespace combined {
namespace model {

class capsule final : public member<capsule, microphone, hrtf> {
public:
    capsule();

    capsule(const capsule& other);
    capsule(capsule&& other) noexcept;

    capsule& operator=(const capsule& other);
    capsule& operator=(capsule&& other) noexcept;

    void swap(capsule& other) noexcept;

    enum class mode { microphone, hrtf };

    void set_name(std::string name);
    std::string get_name() const;

    void set_mode(mode mode);
    mode get_mode() const;

    microphone microphone;
    hrtf hrtf;

private:
    std::string name_ = "new capsule";
    mode mode_ = mode::microphone;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
