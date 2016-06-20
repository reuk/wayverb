#include "Spectrogram.hpp"

#include "ThumbnailBuffer.hpp"

#include "common/fftwf_helpers.h"
#include "common/sinc.h"

class BufferedSpectrogram::Impl final {
public:
    Impl(size_t buffer_size, size_t window_size, size_t hop_size)
            : input_buffer(buffer_size, window_size, hop_size, 0)
            , complex_size(window_size / 2 + 1)
            , window(blackman(window_size))
            , window_buffer(window_size)
            , i(window_size)
            , o(complex_size)
            , r2c(fftwf_plan_dft_r2c_1d(
                      window_size, i.data(), o.data(), FFTW_ESTIMATE)) {
        std::vector<float> ones(window_size, 1);
        normalisation_factor = 1;
        normalisation_factor = Decibels::decibelsToGain(
                compute_slice(ones.begin(), ones.end()).front() * 2, -200.0f);
    }

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
        std::transform(
                begin, end, window.begin(), i.begin(), [](auto a, auto b) {
                    return a * b;
                });
        fftwf_execute(r2c);
        std::vector<float> ret(complex_size);
        std::transform(o.begin(), o.end(), ret.begin(), [this](auto a) {
            auto ret = ((a[0] * a[0]) + (a[1] * a[1])) / normalisation_factor;
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

BufferedSpectrogram::BufferedSpectrogram(size_t buffer_size,
                                         size_t window_size,
                                         size_t hop_size)
        : pimpl(std::make_unique<Impl>(buffer_size, window_size, hop_size)) {
}

BufferedSpectrogram::BufferedSpectrogram(BufferedSpectrogram&& rhs) noexcept =
        default;
BufferedSpectrogram& BufferedSpectrogram::operator=(
        BufferedSpectrogram&& rhs) noexcept = default;

BufferedSpectrogram::~BufferedSpectrogram() noexcept = default;

bool BufferedSpectrogram::has_waiting_frames() const {
    return pimpl->has_waiting_frames();
}

size_t BufferedSpectrogram::get_window_size() const {
    return pimpl->get_window_size();
}

size_t BufferedSpectrogram::get_hop_size() const {
    return pimpl->get_hop_size();
}

void BufferedSpectrogram::write(const float* begin, const float* end) {
    pimpl->write(begin, end);
}

std::vector<float> BufferedSpectrogram::read_frame() {
    return pimpl->read_frame();
}

//----------------------------------------------------------------------------//

class MultichannelBufferedSpectrogram::Impl final {
public:
    Impl(size_t channels,
         size_t buffer_size,
         size_t window_size,
         size_t hop_size) {
        for (auto i = 0u; i != channels; ++i) {
            spectrograms.emplace_back(buffer_size, window_size, hop_size);
        }
    }

    bool has_waiting_frames() const {
        return spectrograms.front().has_waiting_frames();
    }

    size_t get_num_channels() const {
        return spectrograms.size();
    }

    size_t get_window_size() const {
        return spectrograms.front().get_window_size();
    }

    size_t get_hop_size() const {
        return spectrograms.front().get_hop_size();
    }

    void write(const float** begin, size_t num) {
        auto lim = get_num_channels();
        std::vector<const float*> ends(lim);
        std::transform(begin, begin + lim, ends.begin(), [num](auto i) {
            return i + num;
        });
        write(begin, ends.data());
    }

    void write(const float** begin, const float** end) {
        auto lim = get_num_channels();
        for (auto i = 0u; i != lim; ++i) {
            spectrograms[i].write(begin[i], end[i]);
        }
    }

    std::vector<std::vector<float>> read_frame() {
        auto lim = get_num_channels();
        std::vector<std::vector<float>> ret(lim);
        std::transform(spectrograms.begin(),
                       spectrograms.end(),
                       ret.begin(),
                       [](auto& i) { return i.read_frame(); });
        return ret;
    }

private:
    std::vector<BufferedSpectrogram> spectrograms;
};

//----------------------------------------------------------------------------//

MultichannelBufferedSpectrogram::MultichannelBufferedSpectrogram(
        size_t channels,
        size_t buffer_size,
        size_t window_size,
        size_t hop_size)
        : pimpl(std::make_unique<Impl>(
                  channels, buffer_size, window_size, hop_size)) {
}

MultichannelBufferedSpectrogram::~MultichannelBufferedSpectrogram() noexcept =
        default;

bool MultichannelBufferedSpectrogram::has_waiting_frames() const {
    return pimpl->has_waiting_frames();
}

size_t MultichannelBufferedSpectrogram::get_num_channels() const {
    return pimpl->get_num_channels();
}

size_t MultichannelBufferedSpectrogram::get_window_size() const {
    return pimpl->get_window_size();
}

size_t MultichannelBufferedSpectrogram::get_hop_size() const {
    return pimpl->get_hop_size();
}

void MultichannelBufferedSpectrogram::write(const float** begin, size_t num) {
    pimpl->write(begin, num);
}

void MultichannelBufferedSpectrogram::write(const float** begin,
                                            const float** end) {
    pimpl->write(begin, end);
}

std::vector<std::vector<float>> MultichannelBufferedSpectrogram::read_frame() {
    return pimpl->read_frame();
}