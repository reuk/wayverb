#include "common/decibels.h"
#include "common/fftw/buffer.h"
#include "common/hilbert.h"
#include "common/map_to_vector.h"
#include "common/stl_wrappers.h"
#include "fftw/plan.h"

aligned::vector<std::complex<float>> make_complex(
        const aligned::vector<float>& x) {
    return map_to_vector(x, [](auto i) { return std::complex<float>{i, 0}; });
}

fftwf::buffer<fftwf_complex> ifft(fftwf::buffer<fftwf_complex>& sig) {
    fftwf::buffer<fftwf_complex> ret{sig.size()};
    const fftwf::plan plan{fftwf_plan_dft_1d(
            sig.size(), sig.data(), ret.data(), -1, FFTW_ESTIMATE)};
    fftwf_execute(plan);
    return ret;
}

fftwf::buffer<fftwf_complex> fft(fftwf::buffer<fftwf_complex>& sig) {
    fftwf::buffer<fftwf_complex> ret{sig.size()};
    const fftwf::plan plan{fftwf_plan_dft_1d(
            sig.size(), sig.data(), ret.data(), 1, FFTW_ESTIMATE)};
    fftwf_execute(plan);
    return ret;
}

fftwf::buffer<fftwf_complex> to_buffer(
        const aligned::vector<std::complex<float>>& x) {
    fftwf::buffer<fftwf_complex> ret{x.size()};
    for (auto i{0u}; i != x.size(); ++i) {
        ret.data()[i][0] = x[i].real();
        ret.data()[i][1] = x[i].imag();
    }
    return ret;
}

aligned::vector<std::complex<float>> to_vector(
        const fftwf::buffer<fftwf_complex>& x) {
    aligned::vector<std::complex<float>> ret;
    ret.reserve(x.size());
    for (auto i : x) {
        ret.emplace_back(i[0], i[1]);
    }
    return ret;
}

aligned::vector<std::complex<float>> ifft(
        const aligned::vector<std::complex<float>>& x) {
    auto buf{to_buffer(x)};
    return to_vector(ifft(buf));
}

aligned::vector<std::complex<float>> fft(
        const aligned::vector<std::complex<float>>& x) {
    auto buf{to_buffer(x)};
    return to_vector(fft(buf));
}

//  Walk from begin() + 1 and from end(), adding cplx conjugates.
void fold(fftwf::buffer<fftwf_complex>& buffer) {
    //  This is basically the same as the in-place range reverse algorithm.
    auto forward{buffer.begin() + 1}, backward{buffer.end()};
    while (forward != backward && forward != --backward) {
        //  Add complex conjugate to positive frequency bin.
        (*forward)[0] += (*backward)[0];
        (*forward)[1] -= (*backward)[1];

        //  Zero out the negative frequency bin.
        (*backward)[0] = 0;
        (*backward)[1] = 0;

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
    if (!(buffer.size() % 2)) {
        (*forward)[0] = 0;
        (*forward)[1] = 0;
//        (*forward)[1] = -(*forward)[1];
    }
}

void clipdb(aligned::vector<std::complex<float>>& s, float cutoff) {
    if (s.empty()) {
        throw std::runtime_error("clipdb: s must not be empty");
    }
    if (0 < cutoff) {
        throw std::runtime_error("clipdb: cutoff must not be larger than 0");
    }
    const auto as{map_to_vector(s, [](auto i) { return std::abs(i); })};
    const auto mas{*std::max_element(as.begin(), as.end())};
    const auto thresh{mas * decibels::db2a(cutoff)};  // db to linear
    for (auto i{0u}; i != s.size(); ++i) {
        s[i] = as[i] < thresh ? thresh : s[i];
    }
}

/// Given a complex spectrum, calculate a corresponding minimum-phase version.
/// See: https://ccrma.stanford.edu/~jos/fp/Matlab_listing_mps_m.html
aligned::vector<std::complex<float>> mps(
        aligned::vector<std::complex<float>> spectrum) {
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
    clipdb(spectrum, -100);
    //  log
    fftwf::buffer<fftwf_complex> cplx_buffer{spectrum.size()};
    for (auto i{0u}; i != spectrum.size(); ++i) {
        const auto logged{std::log(spectrum[i])};
        cplx_buffer.data()[i][0] = logged.real();
        cplx_buffer.data()[i][1] = logged.imag();
    }
    //  ifft
    auto inverse{ifft(cplx_buffer)};

    //  fold
    fold(inverse);

    //  fft
    auto ret{to_vector(fft(inverse))};

    //  exp
    for (auto& i : ret) {
        i /= ret.size();
        i = std::exp(i);
    }

    return ret;
}
