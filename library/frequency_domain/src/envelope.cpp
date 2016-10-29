#include "frequency_domain/envelope.h"

namespace frequency_domain {

double max_width_factor(range<double> r, double step) {
    using std::pow;
    const auto base = std::pow(r.get_max() / r.get_min(), step);
    return (base - 1) / (base + 1);
}

double width_factor(range<double> r, size_t bands, double overlap) {
    if (overlap < 0 || 1 < overlap) {
        throw std::runtime_error{"overlap must be in the range 0-1"};
    }
    return max_width_factor(r, 1.0 / bands) * overlap;
}

////////////////////////////////////////////////////////////////////////////////

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
                                  range<double> r,
                                  double width_factor,
                                  size_t l) {
    if (width_factor < 0 || 1 < width_factor) {
        throw std::runtime_error{"width_factor must be between 0 and 1"};
    }

    const auto lower_width = r.get_min() * width_factor;
    const auto upper_width = r.get_max() * width_factor;
    if (frequency < r.get_min() - lower_width ||
        r.get_max() + upper_width <= frequency) {
        return 0;
    }

    const auto lower_p = frequency - r.get_min();
    if (std::abs(lower_p) <= lower_width) {
        return lower_band_edge(r.get_min(), lower_p, lower_width, l);
    }

    const auto upper_p = frequency - r.get_max();
    if (std::abs(upper_p) <= upper_width) {
        return upper_band_edge(r.get_max(), upper_p, upper_width, l);
    }

    return 1;
}

double compute_lopass_magnitude(double frequency,
                                double edge,
                                double width_factor,
                                size_t l) {
    if (width_factor < 0 || 1 < width_factor) {
        throw std::runtime_error{"width_factor must be between 0 and 1"};
    }

    const auto absolute_width = edge * width_factor;
    if (frequency < edge - absolute_width) {
        return 1;
    }
    if (edge + absolute_width <= frequency) {
        return 0;
    }
    return upper_band_edge(edge, frequency - edge, absolute_width, l);
}

double compute_hipass_magnitude(double frequency,
                                double edge,
                                double width_factor,
                                size_t l) {
    if (width_factor < 0 || 1 < width_factor) {
        throw std::runtime_error{"width_factor must be between 0 and 1"};
    }

    const auto absolute_width = edge * width_factor;
    if (frequency < edge - absolute_width) {
        return 0;
    }
    if (edge + absolute_width <= frequency) {
        return 1;
    }
    return lower_band_edge(edge, frequency - edge, absolute_width, l);
}

}  // namespace frequency_domain
