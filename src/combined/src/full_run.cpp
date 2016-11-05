#include "combined/full_run.h"

namespace wayverb {
namespace combined {

std::experimental::optional<util::aligned::vector<util::aligned::vector<float>>>
full_run(const engine& engine,
         const util::aligned::vector<std::unique_ptr<capsule_base>>& capsules,
         double sample_rate,
         const std::atomic_bool& keep_going,
         const engine::state_callback& callback) {
    const auto intermediate = engine.run(keep_going, callback);

    if (intermediate == nullptr) {
        return std::experimental::nullopt;
    }

    callback(state::postprocessing, 1.0);

    util::aligned::vector<util::aligned::vector<float>> channels;
    for (auto it = begin(capsules), e = end(capsules); it != e && keep_going;
         ++it) {
        channels.emplace_back((*it)->postprocess(*intermediate, sample_rate));
    }

    if (!keep_going) {
        return std::experimental::nullopt;
    }

    return channels;
}

}  // namespace combined
}  // namespace wayverb
