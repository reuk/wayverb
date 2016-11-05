#pragma once

#include "combined/capsules.h"

#include <experimental/optional>

namespace wayverb {
namespace combined {

/// Runs the engine, then immediately processes the results using a set of
/// capsules.
std::experimental::optional<util::aligned::vector<util::aligned::vector<float>>>
full_run(const engine& engine,
         const util::aligned::vector<std::unique_ptr<capsule_base>>& capsules,
         double sample_rate,
         const std::atomic_bool& keep_going,
         const engine::state_callback& callback);

////////////////////////////////////////////////////////////////////////////////

}  // namespace combined
}  // namespace wayverb
