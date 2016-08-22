#include "common/model/receiver_settings.h"

#include "common/stl_wrappers.h"

namespace model {

glm::vec3 get_pointing(const Pointer& p, const glm::vec3& position) {
    switch (p.mode) {
        case Pointer::Mode::spherical: return compute_pointing(p.spherical);
        case Pointer::Mode::look_at:
            return glm::normalize(p.look_at - position);
    }
}

aligned::vector<glm::vec3> get_pointing(const ReceiverSettings& u) {
    switch (u.mode) {
        case ReceiverSettings::Mode::microphones: {
            aligned::vector<glm::vec3> ret(u.microphones.size());
            proc::transform(u.microphones, ret.begin(), [&](const auto& i) {
                return get_pointing(i.pointer, u.position);
            });
            return ret;
        }
        case ReceiverSettings::Mode::hrtf: {
            return aligned::vector<glm::vec3>{get_pointing(u.hrtf, u.position)};
        }
    }
}

}  // namespace model
