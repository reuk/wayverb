#include "combined/model/capsule_presets.h"

namespace wayverb {
namespace combined {
namespace model {

/// A capsule preset must have at least one capsule, but there may be no
/// presets.
const std::vector<capsule_preset> capsule_presets{
        capsule_preset{"mono omni",
                       {capsule{"omni", microphone{core::orientation{}, 0.0}}}},

        capsule_preset{"binaural pair",
                       {capsule{"left ear",
                                hrtf{core::orientation{{-1, 0, 0}},
                                     core::attenuator::hrtf::channel::left}},
                        capsule{"right ear",
                                hrtf{core::orientation{{1, 0, 0}},
                                     core::attenuator::hrtf::channel::right}}}},

        capsule_preset{
                "xy pair",
                {capsule{"left cardioid",
                         microphone{core::orientation{{-1, 0, 1}}, 0.5}},
                 capsule{"right cardioid",
                         microphone{core::orientation{{1, 0, 1}}, 0.5}}}},

        capsule_preset{"blumlein pair",
                       {capsule{"left bi",
                                microphone{core::orientation{{1, 0, 1}}, 1}},
                        capsule{"right bi",
                                microphone{core::orientation{{-1, 0, 1}}, 1}}}},

};

}  // namespace model
}  // namespace combined
}  // namespace wayverb

