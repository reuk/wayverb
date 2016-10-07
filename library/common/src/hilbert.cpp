#include "common/hilbert.h"

#include "utilities/decibels.h"
#include "utilities/map_to_vector.h"

#include "frequency_domain/fft.h"

//  Walk from begin() + 1 and from end(), adding cplx conjugates.
template <typename It>
void fold(It begin, It end) {
    const auto range_size{std::distance(begin, end)};

    if (!range_size) {
        throw std::runtime_error{"can't fold empty range"};
    }
    //  This is basically the same as the in-place range reverse algorithm.
    auto forward{begin + 1}, backward{end};
    while (forward != backward && forward != --backward) {
        //  Add complex conjugate to positive frequency bin.
        *forward += conj(*backward);

        //  Zero out the negative frequency bin.
        *backward = 0;

        //  Increment forward iterator.
        ++forward;
    }

    //  We might need to manually set the 'middle' value.
    //  TODO the JOS matlab code for this is a bit weird - it replaces the
    //  middle value with its complex conjugate, which (I think) produces
    //  wrong results (i.e. a signal of [0 1 0 0 0 0 0 0] produces a 'minimum
    //  phase' spectrum with non-constant phase).
    //  Other options are adding the conjugate or zeroing the bin. Both seem to
    //  work... Zeroing the bin intuitively seems more correct though, so I'm
    //  doing that.
    if (!(range_size % 2)) {
        *forward = 0;
        //        (*forward)[1] = -(*forward)[1];
    }
}

template <typename It>
void clipdb(It it, It end, float cutoff) {
    const auto range_size{std::distance(it, end)};

    using std::abs;

    if (!range_size) {
        throw std::runtime_error("clipdb: range must not be empty");
    }
    if (0 < cutoff) {
        throw std::runtime_error("clipdb: cutoff must not be larger than 0");
    }
    const auto mas{std::abs(*std::max_element(it, end, [](auto a, auto b) {
        return abs(a) < abs(b);
    }))};
    const auto thresh{mas * decibels::db2a(cutoff)};  // db to linear
    for (; it != end; ++it) {
        if (abs(*it) < thresh) {
            *it = thresh;
        }
    }
}

/// Given a complex spectrum, calculate a corresponding minimum-phase version.
/// See: https://ccrma.stanford.edu/~jos/fp/Matlab_listing_mps_m.html
std::vector<std::complex<float>> mps(
        std::vector<std::complex<float>> spectrum) {
    if (spectrum.empty()) {
        throw std::runtime_error("mps: input spectrum is empty");
    }
    const auto max{std::max_element(
            spectrum.begin(), spectrum.end(), [](auto i, auto j) {
                return std::abs(i) < std::abs(j);
            })};
    if (std::abs(*max) == 0) {
        throw std::runtime_error("mps: spectrum is completely zeroed");
    }

    //  in matlab land: sm = exp( fft( fold( ifft( log( clipdb(s,-100) )))));

    //  clipdb
    clipdb(spectrum.begin(), spectrum.end(), -100);
    //  log
    for (auto& i : spectrum) {
        i = std::log(i);
    }

    frequency_domain::dft_1d ifft{
            frequency_domain::dft_1d::direction::backwards, spectrum.size()};
    auto inverse{run(ifft, spectrum.data(), spectrum.data() + spectrum.size())};

    //  fold
    fold(inverse.begin(), inverse.end());

    //  fft
    frequency_domain::dft_1d fft{frequency_domain::dft_1d::direction::forwards,
                                 inverse.size()};
    auto ret{run(fft, inverse.data(), inverse.data() + inverse.size())};

    //  exp
    for (auto& i : ret) {
        i /= ret.size();
        i = std::exp(i);
    }

    return ret;
}
