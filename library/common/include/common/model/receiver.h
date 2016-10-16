#pragma once

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/model/microphone.h"
#include "common/orientable.h"

#include "utilities/aligned/vector.h"
#include "utilities/mapping_iterator_adapter.h"

#include <vector>

namespace model {

struct receiver final {
    glm::vec3 position{0, 0, 0};

    enum class mode { microphones, hrtf };
    mode mode{mode::microphones};

    template <enum mode M>
    using mode_t = std::integral_constant<enum mode, M>;

    aligned::vector<microphone> microphones{microphone{}};
    orientable hrtf{};
};

aligned::vector<glm::vec3> get_pointing(const receiver& u);

//----------------------------------------------------------------------------//

struct microphone_mapper final {
    glm::vec3 position;

    template <typename T>
    auto operator()(const T& t) const {
        return ::microphone{get_pointing(t.orientable, position), t.shape};
    }
};

struct hrtf_mapper final {
    glm::vec3 pointing;

    template <typename T>
    auto operator()(const T& t) const {
        return ::hrtf{pointing, glm::vec3{0, 1, 0}, t};
    }
};

template <typename It>
auto make_microphone_iterator(It it, const receiver& r) {
    return make_mapping_iterator_adapter(std::move(it),
                                         microphone_mapper{r.position});
}

template <typename It>
auto make_hrtf_iterator(It it, const receiver& r) {
    return make_mapping_iterator_adapter(
            std::move(it), hrtf_mapper{get_pointing(r.hrtf, r.position)});
}

//----------------------------------------------------------------------------//

inline auto get_begin(const receiver& r,
                      receiver::mode_t<receiver::mode::microphones>) {
    return make_microphone_iterator(begin(r.microphones), r);
}

inline auto get_end(const receiver& r,
                    receiver::mode_t<receiver::mode::microphones>) {
    return make_microphone_iterator(end(r.microphones), r);
}

inline auto get_begin(const receiver& r,
                      receiver::mode_t<receiver::mode::hrtf>) {
    return make_hrtf_iterator(begin(hrtf::channels), r);
}

inline auto get_end(const receiver& r, receiver::mode_t<receiver::mode::hrtf>) {
    return make_hrtf_iterator(end(hrtf::channels), r);
}

}  // namespace model
