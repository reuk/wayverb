#include "waveguide/frequency_domain_envelope.h"

#include <algorithm>

namespace wayverb {
namespace waveguide {

bool frequency_domain_envelope::check_invariant() const {
    return std::is_sorted(make_frequency_iterator(cbegin()),
                          make_frequency_iterator(cend()));
}

void frequency_domain_envelope::throw_if_invariant_violated() const {
    if (!check_invariant()) {
        throw std::runtime_error{
                "Frequency_domain_envelope invariant violated."};
    }
}

std::vector<frequency_domain_envelope::point>::const_iterator
frequency_domain_envelope::cbegin() const {
    return points.cbegin();
}
std::vector<frequency_domain_envelope::point>::const_iterator
frequency_domain_envelope::cend() const {
    return points.cend();
}

void frequency_domain_envelope::insert(point p) {
    const auto it =
            std::lower_bound(begin(points), end(points), p, compare_points);
    points.insert(it, p);
    throw_if_invariant_violated();
}

frequency_domain_envelope::const_iterator frequency_domain_envelope::erase(
        const_iterator b, const_iterator e) {
    const auto ret = points.erase(b, e);
    throw_if_invariant_violated();
    return ret;
}

void remove_outside_frequency_range(frequency_domain_envelope& env,
                                    util::range<double> r) {
    /// Remove from the beginning to the first equal/greater frequency relative
    /// to the range minimum.
    env.erase(env.cbegin(),
              std::lower_bound(env.cbegin(),
                               env.cend(),
                               r.get_min(),
                               [](const auto& i, const auto& j) {
                                   return get_frequency(i) < j;
                               }));

    /// Remove from the first greater frequency relative to the range maximum
    /// to the end.
    env.erase(std::upper_bound(env.cbegin(),
                               env.cend(),
                               r.get_max(),
                               [](const auto& i, const auto& j) {
                                   return i < get_frequency(j);
                               }),
              env.cend());
}

}  // namespace waveguide
}  // namespace wayverb
