#include "combined/model/presets/capsule.h"

namespace wayverb {
namespace combined {
namespace model {
namespace presets {

/// A capsule preset must have at least one capsule, but there may be no
/// presets.
const std::vector<capsule> capsules{
        capsule{"mono omni",
                {wayverb::combined::model::capsule{
                        "omni", microphone{core::orientation{}, 0.0}}}},

        capsule{"binaural pair",
                {wayverb::combined::model::capsule{
                         "left ear",
                         hrtf{core::orientation{{-1, 0, 0}},
                              core::attenuator::hrtf::channel::left}},
                 wayverb::combined::model::capsule{
                         "right ear",
                         hrtf{core::orientation{{1, 0, 0}},
                              core::attenuator::hrtf::channel::right}}}},

        capsule{"xy pair",
                {wayverb::combined::model::capsule{
                         "left cardioid",
                         microphone{core::orientation{{-1, 0, 1}}, 0.5}},
                 wayverb::combined::model::capsule{
                         "right cardioid",
                         microphone{core::orientation{{1, 0, 1}}, 0.5}}}},

        capsule{"blumlein pair",
                {wayverb::combined::model::capsule{
                         "left bi",
                         microphone{core::orientation{{1, 0, 1}}, 1}},
                 wayverb::combined::model::capsule{
                         "right bi",
                         microphone{core::orientation{{-1, 0, 1}}, 1}}}},

};

}  // namespace presets
}  // namespace model
}  // namespace combined
}  // namespace wayverb

