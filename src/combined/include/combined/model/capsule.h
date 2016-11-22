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
    enum class mode { microphone = 1, hrtf };

    using microphone_t = class microphone;
    using hrtf_t = class hrtf;

    explicit capsule(std::string name = "new capsule",
                     mode mode = mode::microphone,
                     microphone_t microphone = microphone_t{},
                     hrtf_t hrtf = hrtf_t{});
    capsule(std::string name, microphone_t);
    capsule(std::string name, hrtf_t);

    void set_name(std::string name);
    std::string get_name() const;

    void set_mode(mode mode);
    mode get_mode() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::base_class<base_type>(this), name_, mode_);
    }

    const auto& microphone() const { return get<0>(); }
    const auto& hrtf() const { return get<1>(); }

    NOTIFYING_COPY_ASSIGN_DECLARATION(capsule)
private:
    void swap(capsule& other) noexcept {
        using std::swap;
        swap(name_, other.name_);
        swap(mode_, other.mode_);
    }

    std::string name_;
    mode mode_;
};

bool operator==(const capsule& a, const capsule& b);
bool operator!=(const capsule& a, const capsule& b);

core::orientation get_orientation(const capsule& capsule);

}  // namespace model
}  // namespace combined
}  // namespace wayverb
