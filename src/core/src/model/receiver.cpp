#include "core/model/receiver.h"

#include "utilities/map_to_vector.h"

namespace core {
namespace model {

util::aligned::vector<glm::vec3> get_pointing(const receiver& u) {
    switch (u.mode) {
        case receiver::mode::microphones: {
            return util::map_to_vector(begin(u.microphones),
                                       end(u.microphones),
                                       [&](const auto& i) {
                                           return get_pointing(i.orientable,
                                                               u.position);
                                       });
        }

        case receiver::mode::hrtf: {
            return util::aligned::vector<glm::vec3>{
                    get_pointing(u.hrtf, u.position)};
        }
    }
}

}  // namespace model
}  // namespace core
