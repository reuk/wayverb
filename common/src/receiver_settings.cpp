#include "common/receiver_settings.h"

#include "common/stl_wrappers.h"

namespace model {

glm::vec3 Pointer::get_pointing(const glm::vec3& position) const {
    switch (mode) {
        case Mode::spherical: return Orientable::compute_pointing(spherical);
        case Mode::look_at: return glm::normalize(look_at - position);
    }
}

aligned::vector<glm::vec3> ReceiverSettings::get_pointing() const {
    switch (mode) {
        case Mode::microphones: {
            aligned::vector<glm::vec3> ret(microphones.size());
            proc::transform(microphones, ret.begin(), [this](const auto& i) {
                return i.pointer.get_pointing(position);
            });
            return ret;
        }
        case Mode::hrtf: {
            return aligned::vector<glm::vec3>{hrtf.get_pointing(position)};
        }
    }
}

}  // namespace model