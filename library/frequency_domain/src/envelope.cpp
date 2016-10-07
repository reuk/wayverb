#include "frequency_domain/envelope.h"

namespace frequency_domain {

double band_edge_impl(double centre, double p, double P, size_t l) {
    return l ? std::sin(M_PI * band_edge_impl(centre, p, P, l - 1) / 2)
             : (((p / P) + 1) / 2);
}

double lower_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::sin(M_PI * band_edge_impl(centre, p, P, l) / 2);
}

double upper_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::cos(M_PI * band_edge_impl(centre, p, P, l) / 2);
}

double band_edge_frequency(int band, size_t bands, double min, double max) {
    return min * std::pow(max / min, band / static_cast<double>(bands));
}

double compute_bandpass_magnitude(double frequency,
                                  double min,
                                  double max,
                                  double lower_edge_width,
                                  double upper_edge_width,
                                  size_t l) {
    if (frequency < min - lower_edge_width ||
        min + upper_edge_width <= frequency) {
        return 0;
    }

    const auto lower_p{frequency - min};
    if (-lower_edge_width <= lower_p && lower_p < lower_edge_width) {
        return lower_band_edge(min, lower_p, lower_edge_width, l);
    }

    const auto upper_p{frequency - max};
    if (-upper_edge_width <= upper_p && upper_p < upper_edge_width) {
        return upper_band_edge(max, upper_p, upper_edge_width, l);
    }

    return 1;
}

double compute_lopass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l) {
    if (frequency < cutoff - width) {
        return 1;
    }
    if (cutoff + width <= frequency) {
        return 0;
    }
    return upper_band_edge(cutoff, frequency - cutoff, width, l);
}

double compute_hipass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l) {
    if (frequency < cutoff - width) {
        return 0;
    }
    if (cutoff + width <= frequency) {
        return 1;
    }
    return lower_band_edge(cutoff, frequency - cutoff, width, l);
}

}  // namespace frequency_domain
