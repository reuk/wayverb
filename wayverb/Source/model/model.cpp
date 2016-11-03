#include "model.h"

#include "utilities/aligned/vector.h"

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

////////////////////////////////////////////////////////////////////////////////

SingleShot get_single_shot(const App& a, size_t input, size_t output) {
    return SingleShot{a.filter_frequency,
                      a.oversample_ratio,
                      a.speed_of_sound,
                      a.rays,
                      a.source[input],
                      a.receiver[output]};
}

util::aligned::vector<SingleShot> get_all_input_output_combinations(
        const App& a) {
    util::aligned::vector<SingleShot> ret;
    ret.reserve(a.source.size() * a.receiver.size());
    for (const auto& i : a.source) {
        for (const auto& j : a.receiver) {
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
