#include "combined/model/capsule.h"

namespace wayverb {
namespace combined {
namespace model {

capsule::capsule() { connect_all(microphone, hrtf); }

void capsule::set_name(std::string name) {
    name_ = name;
    notify();
}

void capsule::set_mode(mode mode) {
    mode_ = mode;
    notify();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
