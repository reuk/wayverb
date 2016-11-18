#pragma once

#include "combined/model/capsule.h"

#include <vector>

namespace wayverb {
namespace combined {
namespace model {
namespace presets {

struct capsule final {
    std::string name;
    std::vector<wayverb::combined::model::capsule> capsules;
};

/// A capsule preset must have at least one capsule, but there may be no
/// presets.
extern const std::vector<capsule> capsules;

}  // namespace presets
}  // namespace model
}  // namespace combined
}  // namespace wayverb

