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

    void set_name(std::string name);
    std::string get_name() const;

    void set_mode(mode mode);
    mode get_mode() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(cereal::base_class<base_type>(this), name_, mode_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(cereal::base_class<base_type>(this), name_, mode_);
    }

    using microphone_t = shared_value<class microphone>;
    using hrtf_t = shared_value<class hrtf>;

    microphone_t& microphone();
    const microphone_t& microphone() const;

    hrtf_t& hrtf();
    const hrtf_t& hrtf() const;

private:
    std::string name_ = "new capsule";
    mode mode_ = mode::microphone;
};

core::orientation get_orientation(const capsule& capsule);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
