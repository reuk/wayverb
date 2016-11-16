#pragma once

#include "combined/model/capsule.h"
#include "combined/model/vector.h"

namespace wayverb {
namespace combined {
namespace model {

struct capsule_preset final {
    std::string name;
    std::vector<capsule> capsules;
};

/// A capsule preset must have at least one capsule, but there may be no presets.
extern const std::vector<capsule_preset> capsule_presets;

}  // namespace model
}  // namespace combined
}  // namespace wayverb

