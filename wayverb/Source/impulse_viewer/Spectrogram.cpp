#include "Spectrogram.hpp"

#include "ThumbnailBuffer.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"

class InputBufferedSpectrogram::Impl {
public:
    Impl(size_t buffer_size, size_t window_size, size_t hop_size)
            : input_buffer(buffer_size, window_size, hop_size, 0)
            , complex_size(window_size / 2 + 1)
            , window(blackman(window_size))
            , window_buffer(window_size)
            , i(fftwf_alloc_real(window_size))
            , o(fftwf_alloc_complex(complex_size))
            , r2c(fftwf_plan_dft_r2c_1d(
                      window_size, i.get(), o.get(), FFTW_ESTIMATE)) {
        std::vector<float> ones(window_size, 1);
        normalisation_factor = 1;
        normalisation_factor = Decibels::decibelsToGain(
                compute_slice(ones.begin(), ones.end()).front() * 2, -200.0f);
    }
    virtual ~Impl() noexcept = default;

    bool has_waiting_frames() const {
        return input_buffer.has_waiting_frames();
    }

    size_t get_window_size() const {
        return input_buffer.get_window_size();
    }

    size_t get_hop_size() const {
        return input_buffer.get_hop_size();
    }

    template <typename It>
    void write(It begin, It end) {
        input_buffer.write(begin, end);
    }

    std::vector<float> read_frame() {
        input_buffer.read(window_buffer.begin(), window_buffer.end());
        return compute_slice(window_buffer.begin(), window_buffer.end());
    }

private:
    template <typename It>
    std::vector<float> compute_slice(It begin, It end) {
        assert(std::distance(begin, end) == input_buffer.get_window_size());
        std::transform(begin, end, window.begin(), i.get(), [](auto a, auto b) {
            return a * b;
        });
        fftwf_execute(r2c);
        std::vector<float> ret(complex_size);
        std::transform(
                o.get(), o.get() + complex_size, ret.begin(), [this](auto a) {
                    auto ret = ((a[0] * a[0]) + (a[1] * a[1])) /
                               normalisation_factor;
                    ret = Decibels::gainToDecibels(ret, -200.0f) * 0.5;
                    return ret;
                });
        return ret;
    }

    InputBufferedHopBuffer<float> input_buffer;
    size_t complex_size;

    std::vector<float> window;
    std::vector<float> window_buffer;
    float normalisation_factor;

    fftwf_r i;
    fftwf_c o;
    FftwfPlan r2c;
};

//----------------------------------------------------------------------------//

InputBufferedSpectrogram::InputBufferedSpectrogram(size_t buffer_size,
                                                   size_t window_size,
                                                   size_t hop_size)
        : pimpl(std::make_unique<Impl>(buffer_size, window_size, hop_size)) {
}

InputBufferedSpectrogram::~InputBufferedSpectrogram() noexcept = default;

bool InputBufferedSpectrogram::has_waiting_frames() const {
    return pimpl->has_waiting_frames();
}

size_t InputBufferedSpectrogram::get_window_size() const {
    return pimpl->get_window_size();
}

size_t InputBufferedSpectrogram::get_hop_size() const {
    return pimpl->get_hop_size();
}

void InputBufferedSpectrogram::write(const float* begin, const float* end) {
    pimpl->write(begin, end);
}

std::vector<float> InputBufferedSpectrogram::read_frame() {
    return pimpl->read_frame();
}