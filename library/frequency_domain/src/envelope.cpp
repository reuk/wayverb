#include "frequency_domain/envelope.h"

namespace frequency_domain {

double max_width_factor(range<double> r, double step) {
    using std::pow;
    const auto base = std::pow(r.get_max() / r.get_min(), step);
    return (base - 1) / (base + 1);
}

double band_edge_impl(double centre, double p, double P, size_t l) {
    return l ? std::sin(M_PI * band_edge_impl(centre, p, P, l - 1) / 2)
             : (((p / P) + 1) / 2);
}

double lower_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::pow(std::sin(M_PI * band_edge_impl(centre, p, P, l) / 2), 2.0);
}

double upper_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::pow(std::cos(M_PI * band_edge_impl(centre, p, P, l) / 2), 2.0);
}

double band_edge_frequency(size_t band, size_t bands, range<double> r) {
    return r.get_min() * std::pow(r.get_max() / r.get_min(),
                                  band / static_cast<double>(bands));
}

double band_centre_frequency(size_t band, size_t bands, range<double> r) {
    return band_edge_frequency(band * 2 + 1, bands * 2, r);
}

double compute_bandpass_magnitude(double frequency,
                                  edge_and_width lower,
                                  edge_and_width upper,
                                  size_t l) {
    if (frequency < lower.edge - lower.width ||
        upper.edge + upper.width <= frequency) {
        return 0;
    }

    const auto lower_p = frequency - lower.edge;
    if (-lower.width < lower_p && lower_p <= lower.width) {
        return lower_band_edge(lower.edge, lower_p, lower.width, l);
    }

    const auto upper_p = frequency - upper.edge;
    if (-upper.width < upper_p && upper_p <= upper.width) {
        return upper_band_edge(upper.edge, upper_p, upper.width, l);
    }

    return 1;
}

double compute_lopass_magnitude(double frequency,
                                edge_and_width cutoff,
                                size_t l) {
    if (frequency < cutoff.edge - cutoff.width) {
        return 1;
    }
    if (cutoff.edge + cutoff.width <= frequency) {
        return 0;
    }
    return upper_band_edge(
            cutoff.edge, frequency - cutoff.edge, cutoff.width, l);
}

double compute_hipass_magnitude(double frequency,
                                edge_and_width cutoff,
                                size_t l) {
    if (frequency < cutoff.edge - cutoff.width) {
        return 0;
    }
    if (cutoff.edge + cutoff.width <= frequency) {
        return 1;
    }
    return lower_band_edge(
            cutoff.edge, frequency - cutoff.edge, cutoff.width, l);
}

}  // namespace frequency_domain
