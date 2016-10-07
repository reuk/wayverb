#pragma once

#include <complex>
#include <memory>
#include <vector>

namespace frequency_domain {

class dft_1d final {
public:
    enum class direction { forwards, backwards };

    dft_1d(direction dir, size_t size);
    ~dft_1d() noexcept;

    dft_1d(const dft_1d&) = delete;
    dft_1d(dft_1d&&) noexcept = delete;

    dft_1d& operator=(const dft_1d&) = delete;
    dft_1d& operator=(dft_1d&&) noexcept = delete;

    std::vector<std::complex<float>> run(
            const std::complex<float>* begin,
            const std::complex<float>* end);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

std::vector<std::complex<float>> run(dft_1d& fft,
                                     const std::complex<float>* begin,
                                     const std::complex<float>* end);
std::vector<std::complex<float>> run(dft_1d& fft,
                                     const float* begin,
                                     const float* end);

}  // namespace frequency_domain
