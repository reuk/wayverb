#include "model.h"

#include "utilities/aligned/vector.h"

namespace model {

util::aligned::vector<SingleShot> get_all_input_output_combinations(
        const App& a) {
    util::aligned::vector<SingleShot> ret;
    ret.reserve(a.sources.size() * a.receivers.size());
    for (const auto& i : a.sources) {
        for (const auto& j : a.receivers) {
            ret.emplace_back(SingleShot{a.filter_frequency,
                                        a.oversample_ratio,
                                        a.speed_of_sound,
                                        a.rays,
                                        i,
                                        j});
        }
    }
    return ret;
}

}  // namespace model
