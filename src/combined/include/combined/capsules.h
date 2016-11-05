#pragma once

#include "combined/engine.h"

#include "core/scene_data_loader.h"
#include "core/serialize/attenuators.h"

namespace wayverb {
namespace combined {

class capsule_base {
public:
    capsule_base() = default;
    capsule_base(const capsule_base&) = default;
    capsule_base(capsule_base&&) noexcept = default;
    capsule_base& operator=(const capsule_base&) = default;
    capsule_base& operator=(capsule_base&&) noexcept = default;
    virtual ~capsule_base() noexcept = default;

    virtual std::unique_ptr<capsule_base> clone() const = 0;
    virtual util::aligned::vector<float> postprocess(
            const intermediate& intermediate,
            double sample_rate) const = 0;
    virtual glm::vec3 get_pointing() const = 0;
};

template <typename T>
class capsule final : public capsule_base {
public:
    capsule() = default;
    explicit capsule(T attenuator)
            : attenuator_{std::move(attenuator)} {}

    std::unique_ptr<capsule_base> clone() const override {
        return std::make_unique<capsule>(*this);
    }

    util::aligned::vector<float> postprocess(
            const intermediate& intermediate,
            double sample_rate) const override {
        return intermediate.postprocess(attenuator_, sample_rate);
    }

    glm::vec3 get_pointing() const override {
        return attenuator_.get_pointing();
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("attenuator", attenuator_));
    }

private:
    T attenuator_;
};

template <typename T>
auto make_capsule_ptr(T attenuator) {
    return std::make_unique<capsule<T>>(std::move(attenuator));
}

}  // namespace combined
}  // namespace wayverb
