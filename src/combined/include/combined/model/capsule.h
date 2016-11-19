#pragma once

#include "combined/model/hrtf.h"
#include "combined/model/microphone.h"
#include "combined/model/vector.h"

#include "cereal/types/base_class.hpp"

namespace wayverb {
namespace combined {
namespace model {

class capsule final : public owning_member<capsule, microphone, hrtf> {
public:
    enum class mode { microphone, hrtf };

    using microphone_t = class microphone;
    using hrtf_t = class hrtf;

    capsule(std::string name, mode mode, microphone_t, hrtf_t);
    capsule(std::string name, microphone_t);
    capsule(std::string name, hrtf_t);
    capsule() = default;

    void set_name(std::string name);
    std::string get_name() const;

    void set_mode(mode mode);
    mode get_mode() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::base_class<base_type>(this), name_, mode_);
    }

    shared_value<microphone_t>& microphone();
    const shared_value<microphone_t>& microphone() const;

    shared_value<hrtf_t>& hrtf();
    const shared_value<hrtf_t>& hrtf() const;

private:
    std::string name_ = "new capsule";
    mode mode_ = mode::microphone;
};

core::orientation get_orientation(const capsule& capsule);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
