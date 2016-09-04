#include "common/model/receiver_settings.h"

#include "common/stl_wrappers.h"

namespace model {

glm::vec3 get_pointing(const orientable& p, const glm::vec3& position) {
    switch (p.mode) {
        case orientable::mode::spherical: return compute_pointing(p.spherical);
        case orientable::mode::look_at:
            return glm::normalize(p.look_at - position);
    }
}

aligned::vector<glm::vec3> get_pointing(const receiver_settings& u) {
    switch (u.mode) {
        case receiver_settings::mode::microphones: {
            aligned::vector<glm::vec3> ret(u.microphones.size());
            proc::transform(u.microphones, ret.begin(), [&](const auto& i) {
                return get_pointing(i.orientable, u.position);
            });
            return ret;
        }
        case receiver_settings::mode::hrtf: {
            return aligned::vector<glm::vec3>{get_pointing(u.hrtf, u.position)};
        }
    }
}

}  // namespace model
