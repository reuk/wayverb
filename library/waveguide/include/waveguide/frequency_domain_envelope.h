#pragma once

#include "utilities/mapping_iterator_adapter.h"
#include "utilities/range.h"

#include <vector>

namespace waveguide {

/// Invariant: points are in ascending frequency order.
class frequency_domain_envelope final {
public:
    struct point final {
        double frequency;
        double amplitude;
    };

    using const_iterator = std::vector<point>::const_iterator;

    const_iterator cbegin() const;
    const_iterator cend() const;

    void insert(point p);
    const_iterator erase(const_iterator b, const_iterator e);

    void enforce_minimum_spacing(double spacing);

private:
    bool check_invariant() const;
    void throw_if_invariant_violated() const;

    std::vector<point> points;
};

template <typename It>
void add_points(frequency_domain_envelope& env, It b, It e) {
    for (; b != e; ++b) {
        env.insert(*b);
    }
}

constexpr double get_frequency(const frequency_domain_envelope::point& t) {
    return t.frequency;
}

constexpr double get_amplitude(const frequency_domain_envelope::point& t) {
    return t.amplitude;
}

constexpr bool compare_points(const frequency_domain_envelope::point& a,
                              const frequency_domain_envelope::point& b) {
    return get_frequency(a) < get_frequency(b);
}

template <typename It>
constexpr auto make_frequency_iterator(It it) {
    return make_mapping_iterator_adapter(std::move(it), get_frequency);
}

template <typename It>
constexpr auto make_amplitude_iterator(It it) {
    return make_mapping_iterator_adapter(std::move(it), get_amplitude);
}

void remove_outside_frequency_range(frequency_domain_envelope& env,
                                    range<double> r);

template <size_t N>
auto make_frequency_domain_envelope(const std::array<double, N>& frequency,
                                    const std::array<double, N>& amplitude) {
    frequency_domain_envelope ret;

    for (auto i = 0; i != N; ++i) {
        ret.insert(
                frequency_domain_envelope::point{frequency[i], amplitude[i]});
    }

    return ret;
}

}  // namespace waveguide
