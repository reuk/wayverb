#pragma once

#include "core/orientable.h"
#include "core/scene_data_loader.h"

#include "utilities/mapping_iterator_adapter.h"

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

    virtual util::aligned::vector<float> postprocess(
            const wayverb::combined::intermediate& intermediate,
            double sample_rate) const = 0;
    virtual glm::vec3 get_pointing() const = 0;
};

template <typename T>
class capsule final : public capsule_base {
public:
    capsule(T attenuator)
            : attenuator_{std::move(attenuator)} {}

    util::aligned::vector<float> postprocess(
            const wayverb::combined::intermediate& intermediate,
            double sample_rate) const override {
        return intermediate.postprocess(attenuator_, sample_rate);
    }

    glm::vec3 get_pointing() const override {
        return attenuator_.get_pointing();
    }

private:
    T attenuator_;
};

template <typename T>
auto make_capsule_ptr(T attenuator) {
    return std::make_unique<capsule<T>>(std::move(attenuator));
}

////////////////////////////////////////////////////////////////////////////////

struct capsules final {
    glm::vec3 position;
    wayverb::core::orientable orientable;
    std::vector<std::unique_ptr<capsule_base>> capsules;
};

////////////////////////////////////////////////////////////////////////////////

struct SingleShot final {
    float filter_frequency;
    float oversample_ratio;
    float speed_of_sound;
    size_t rays;
    glm::vec3 source;
    capsules receiver;
};

struct App final {
    float filter_frequency{500};
    float oversample_ratio{2};
    float speed_of_sound{340};
    size_t rays{100000};
    std::vector<glm::vec3> source{glm::vec3{0}};
    std::vector<capsules> receiver{};
};

SingleShot get_single_shot(const App& a, size_t input, size_t output);
util::aligned::vector<SingleShot> get_all_input_output_combinations(
        const App& a);

struct Persistent final {
    App app;
    util::aligned::vector<wayverb::core::scene_data_loader::material> materials;
};

struct RenderState final {
    bool is_rendering{false};
    wayverb::combined::state state{wayverb::combined::state::idle};
    double progress{0};
    bool visualise{true};
};

struct FullModel final {
    Persistent persistent;
    util::aligned::vector<wayverb::core::scene_data_loader::material> presets;
    RenderState render_state;
    int shown_surface{-1};
    bool needs_save{false};
};

}  // namespace model
