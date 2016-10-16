#include "common/model/receiver.h"

#include "utilities/map_to_vector.h"

namespace model {

aligned::vector<glm::vec3> get_pointing(const receiver& u) {
    switch (u.mode) {
        case receiver::mode::microphones: {
            return map_to_vector(begin(u.microphones),
                                 end(u.microphones),
                                 [&](const auto& i) {
                                     return get_pointing(i.orientable,
                                                         u.position);
                                 });
        }

        case receiver::mode::hrtf: {
            return aligned::vector<glm::vec3>{get_pointing(u.hrtf, u.position)};
        }
    }
}

}  // namespace model
