#pragma once

#include <array>
#include <complex>

//  pressure - N / m^2
//  force per unit area
//
//  intensity - mean energy flow
//      energy transported per second through a unit area
//
//  If several pressure signals are present, their total level is the sum of the
//      individual pressures.
//
//  For cohrent, in-phase signals the pressure-time functions are added,
//      doubling the pressure (a level change of 6dB)
//
//  If they're in antiphase, the signals cancel (minus infinity level)
//
//  If they're incorherent, you can sum intensities directly.

namespace raytracer {

//  vorlander2007 eq 3.18
//  pressure contribution from a source (or image source) depends on
//      distance from image source, r
//      angular wavenumber, k
//      the product of reflection functions at incident angles along the path
//      ...
//
//  kuttruff2009 eq 9.23
//  each surface has a reflection factor which is a complex function of
//      frequency
//      angle of incidence
//  i.e. we can produce a complex spectrum for each surface that varies with
//      angle of incidence
//  *I think* we convolve the spectra together, adjust the phase depending on
//  the total distance travelled from the image source, and adjust the level
//  also depending on the distance
//
////////////////////////////////////////////////////////////////////////////////
//
//  OK here's what we'll do
//  -----------------------
//
//  * each surface will define multiband *absorption* coefficients
//  * we can find multiband reflection factor magnitudes easily from these
//      * then for each set of magnitudes, smooth to full spectra and
//        accompanying compute minimum phase spectra
//  * multiply together the reflection factors for multi-reflection paths
//
//  alternatively
//  -------------
//
//  * each surface has a set of impedances (perhaps derived from absorptions)
//  * trace in bands, keeping track of phase changes due to incident angles
//  * extract real signal somehow????

/// This is rubbish but I don't have time to write a proper cubic spline
/// interpolation.
/// y0 and y1 are interpolation values, x is between 0 and 1
float cosine_interpolation(float y0, float y1, float x) {
    const auto phase = (1 - std::cos(x * M_PI)) * 0.5;
    return y0 * (1 - phase) + y1 * phase;
}

//  TODO add an abstraction point for image-source finding
//      i.e. when a valid path is found, delegate the actual action to a
//      callback
//  TODO analytic signal / hilbert function

template <size_t num_bands>
struct img_src_calculation final {
    using real = float;
    using cplx = std::complex<real>;

    template <typename T>
    using bins = std::array<T, num_bands>;

    template <typename T, typename Func>
    static constexpr auto map_bins(const bins<T>& x, Func func) {
        using ret_t = decltype(func(x.front()));
        bins<ret_t> ret;
        using std::begin;
        using std::end;
        std::transform(
                begin(x), end(x), begin(ret), [&](auto i) { return func(i); });
        return ret;
    }

    static auto absorption_to_reflection_factor(real x) {
        return std::sqrt(1 - x);
    }

    static auto absorption_to_reflection_factor(const bins<real>& x) {
        return map_bins(
                x, [](auto i) { return absorption_to_reflection_factor(i); });
    }

    static real compute_phase(real frequency, real time) {
        return 2 * M_PI * frequency * time;
    }

    static auto compute_phase(const bins<real>& per_band_frequencies,
                              real time) {
        return map_bins(per_band_frequencies,
                        [&](auto i) { return compute_phase(i, time); });
    }

    static auto compute_unit_phase_spectrum(
            const bins<real>& per_band_frequencies, real time) {
        const auto phases = compute_phase(per_band_frequencies, time);
        return map_bins(phases, [](auto i) { return std::exp(cplx{0, -i}); });
    }

    template <size_t output_bins>
    static std::array<real, output_bins> cosine_smoothing(
            const bins<real>& per_band_amplitudes,
            const bins<real>& per_band_frequencies,
            real sample_rate) {
        const auto extend = [](auto min, auto arr, auto max) {
            std::array<real, num_bands + 2> extended;
            extended.front() = min;
            extended.back() = max;
            std::copy(arr.begin(), arr.end(), extended.begin() + 1);
            return extended;
        };

        const auto extended_amplitudes = extend(0, per_band_amplitudes, 0);
        const auto extended_frequencies =
                extend(0, per_band_frequencies, sample_rate / 2);

        std::array<real, output_bins> ret;
        auto this_band = 1u;
        for (auto i = 0u; i != ret.size(); ++i) {
            const auto bin_frequency = (i * sample_rate) / (2 * ret.size());
            while (extended_frequencies[this_band] < bin_frequency) {
                this_band += 1;
            }

            const auto lower = extended_frequencies[this_band - 1];
            const auto upper = extended_frequencies[this_band];
            const auto x = (bin_frequency - lower) / (upper - lower);
            ret[i] = cosine_interpolation(extended_amplitudes[this_band - 1],
                                          extended_amplitudes[this_band],
                                          x);
        }

        return ret;
    }

    template <typename It>  /// iterator over absorption coefficients
    static auto compute_pressure_spectrum(
            It begin,
            It end,
            real distance,
            real speed_of_sound,
            real amplitude_adjustment,  //  ambient density * source strength /
                                        //  4 pi
            const bins<real>& per_band_frequencies) {
        //  Find the phase spectrum at this distance.
        const auto time = distance / speed_of_sound;

        auto spectrum = compute_unit_phase_spectrum(per_band_frequencies, time);

        //  Correct spectrum for pressure loss over distance travelled.
        const auto amplitude = amplitude_adjustment / distance;
        for (auto& i : spectrum) {
            i *= amplitude;
        }

        //  Now, for each reflection, compute its spectrum and convolve it with
        //  the current spectrum
        for (; begin != end; ++begin) {
        }

        return spectrum;
    }
};

}  // namespace raytracer
