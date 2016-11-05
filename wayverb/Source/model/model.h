#pragma once

#include "combined/engine.h"

#include "core/scene_data_loader.h"
#include "core/serialize/attenuators.h"

#include <vector>


namespace model {

////////////////////////////////////////////////////////////////////////////////
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
