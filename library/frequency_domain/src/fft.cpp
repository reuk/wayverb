#include "frequency_domain/fft.h"
#include "frequency_domain/buffer.h"

#include "plan.h"

namespace frequency_domain {

using cbuf = buffer<fftwf_complex>;

template <typename It, typename Out>
void copy_to_buffer(It it, It end, Out out) {
    for (; it != end; ++it, ++out) {
        *out[0] = it->real();
        *out[1] = it->imag();
    }
}

template <typename It, typename Out>
void copy_to_vector(It it, It end, Out out) {
    for (; it != end; ++it, ++out) {
        *out = std::complex<float>{*it[0], *it[1]};
    }
}

class dft_1d::impl final {
public:
    impl(dft_1d::direction dir, size_t size)
            : i_buf_{size}
            , o_buf_{size}
            , plan_{fftwf_plan_dft_1d(size,
                                      i_buf_.data(),
                                      o_buf_.data(),
                                      dir == direction::forwards ? 1 : -1,
                                      FFTW_ESTIMATE)} {}

    impl(const impl&) = delete;
    impl(impl&&) = delete;

    impl& operator=(const impl&) = delete;
    impl& operator=(impl&&) = delete;

    template <typename It>
    auto run(It begin, It end) {
        i_buf_.zero();
        copy_to_buffer(begin, end, i_buf_.begin());
        fftwf_execute(plan_);
        std::vector<std::complex<float>> ret(i_buf_.size(), 0);
        copy_to_vector(o_buf_.begin(), o_buf_.end(), ret.begin());
        return ret;
    }

private:
    cbuf i_buf_;
    cbuf o_buf_;
    plan plan_;
};

dft_1d::dft_1d(direction dir, size_t size)
        : pimpl_{std::make_unique<impl>(dir, size)} {}

dft_1d::~dft_1d() noexcept = default;

std::vector<std::complex<float>> dft_1d::run(const std::complex<float>* begin,
                                             const std::complex<float>* end) {
    return pimpl_->run(begin, end);
}

std::vector<std::complex<float>> run(dft_1d& fft,
                                     const std::complex<float>* begin,
                                     const std::complex<float>* end) {
    return fft.run(begin, end);
}

template <typename It>
auto make_complex(It it, It end) {
    std::vector<std::complex<float>> ret{};
    ret.resize(std::distance(it, end));
    for (; it != end; ++it) {
        ret.emplace_back(*it, 0);
    }
    return ret;
}

std::vector<std::complex<float>> run(dft_1d& fft,
                                     const float* begin,
                                     const float* end) {
    const auto cplx{make_complex(begin, end)};
    return run(fft, cplx.data(), cplx.data() + cplx.size());
}

}  // namespace frequency_domain
