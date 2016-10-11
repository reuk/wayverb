#pragma once

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/orientable.h"

#include "utilities/aligned/vector.h"

#include <vector>

namespace model {

struct orientable final {
    enum class mode { spherical, look_at };

    mode mode{mode::spherical};
    az_el spherical{};
    glm::vec3 look_at{0};
};

glm::vec3 get_pointing(const orientable& u, const glm::vec3& position);

//----------------------------------------------------------------------------//

struct microphone final {
    orientable orientable{};
    float shape{0};
};

//----------------------------------------------------------------------------//

struct receiver_settings final {
    enum class mode { microphones, hrtf };

    glm::vec3 position{0, 0, 0};
    mode mode{mode::microphones};
    aligned::vector<microphone> microphones{microphone{}};
    orientable hrtf{};
};

aligned::vector<glm::vec3> get_pointing(const receiver_settings& u);

struct microphone_mapper final {
    glm::vec3 position_;

    template <typename T>
    auto operator()(const T& t) const {
        return ::microphone{get_pointing(t.orientable, position_), t.shape};
    }
};

struct hrtf_mapper final {
    glm::vec3 pointing_;

    template <typename T>
    auto operator()(const T& t) const {
        return ::hrtf{pointing_, glm::vec3{0, 1, 0}, t};
    }
};

template <typename It>
auto make_microphone_iterator(It it, const receiver_settings& r) {
    return make_mapping_iterator_adapter(std::move(it),
                                         microphone_mapper{r.position});
}

template <typename It>
auto make_hrtf_iterator(It it, const receiver_settings& r) {
    return make_mapping_iterator_adapter(
            std::move(it), hrtf_mapper{get_pointing(r.hrtf, r.position)});
}

}  // namespace model
