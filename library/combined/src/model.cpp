#include "combined/model.h"

#include "utilities/aligned/vector.h"

SingleShot get_single_shot(const App& a, size_t input, size_t output) {
    return SingleShot{a.filter_frequency,
                      a.oversample_ratio,
                      a.speed_of_sound,
                      a.rays,
                      a.source[input],
                      a.receiver_settings[output]};
}

aligned::vector<SingleShot> get_all_input_output_combinations(const App& a) {
    aligned::vector<SingleShot> ret;
    ret.reserve(a.source.size() * a.receiver_settings.size());
    for (const auto& i : a.source) {
        for (const auto& j : a.receiver_settings) {
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
