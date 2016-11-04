#pragma once

#include "combined/engine.h"

#include "core/scene_data_loader.h"
#include "core/serialize/attenuators.h"

#include "utilities/mapping_iterator_adapter.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/vector.hpp"

#include <vector>

namespace model {

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
            const wayverb::combined::intermediate& intermediate,
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
            const wayverb::combined::intermediate& intermediate,
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

}  // namespace model

CEREAL_REGISTER_TYPE(model::capsule<wayverb::core::attenuator::hrtf>)
CEREAL_REGISTER_TYPE(model::capsule<wayverb::core::attenuator::microphone>)

CEREAL_REGISTER_POLYMORPHIC_RELATION(model::capsule_base, model::capsule<wayverb::core::attenuator::hrtf>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(model::capsule_base, model::capsule<wayverb::core::attenuator::microphone>)

namespace model {

////////////////////////////////////////////////////////////////////////////////

struct clone_functor final {
    template <typename T>
    auto operator()(T&& t) const {
        return t->clone();
    }
};

struct receiver final {
    receiver() = default;

    receiver(receiver&&) noexcept = default;
    receiver(const receiver& other)
    : position{other.position}
    , orientable{other.orientable}
    , capsules{util::map_to_vector(begin(other.capsules),
                                   end(other.capsules),
                                   clone_functor{})} {}

    receiver& operator=(receiver&&) noexcept = default;
    receiver& operator=(const receiver& other) {
        auto copy = other;
        swap(copy);
        return *this;
    }

    void swap(receiver& other) noexcept {
        using std::swap;
        swap(position, other.position);
        swap(orientable, other.orientable);
        swap(capsules, other.capsules);
    }

    glm::vec3 position;
    wayverb::core::orientable orientable;
    util::aligned::vector<std::unique_ptr<capsule_base>> capsules;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("position", position),
                cereal::make_nvp("orientable", orientable),
                cereal::make_nvp("capsules", capsules));
    }
};

////////////////////////////////////////////////////////////////////////////////

struct SingleShot final {
    float filter_frequency;
    float oversample_ratio;
    float speed_of_sound;
    size_t rays;
    glm::vec3 source;
    receiver receiver;
};

struct App final {
    float filter_frequency{500};
    float oversample_ratio{2};
    float speed_of_sound{340};
    size_t rays{100000};
    std::vector<glm::vec3> sources{glm::vec3{0}};
    std::vector<receiver> receivers{};

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("filter_frequency", filter_frequency),
                cereal::make_nvp("oversample_ratio", oversample_ratio),
                cereal::make_nvp("rays", rays),
                cereal::make_nvp("sources", sources),
                cereal::make_nvp("receivers", receivers));
    }
};

util::aligned::vector<SingleShot> get_all_input_output_combinations(
        const App& a);

struct Persistent final {
    App app;
    util::aligned::vector<wayverb::core::scene_data_loader::material> materials;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("app", app),
                cereal::make_nvp("materials", materials));
    }
};

}  // namespace model
