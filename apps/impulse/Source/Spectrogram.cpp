#include "Spectrogram.hpp"

#include "ThumbnailBuffer.hpp"

#include "UtilityComponents/Lerp.hpp"

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
        normalisation_factor = compute_slice(ones.begin(), ones.end()).front();
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
    void write(It begin, size_t num) {
        input_buffer.write(begin, num);
    }

    std::vector<float> read_frame() {
        input_buffer.read(window_buffer.data(), window_buffer.size());
        auto ret = compute_slice(window_buffer.begin(), window_buffer.end());
        convert_to_db(ret);
        return ret;
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
            return ((a[0] * a[0]) + (a[1] * a[1])) / normalisation_factor;
        });
        return ret;
    }

    void convert_to_db(std::vector<float>& ret) {
        for (auto& i : ret) {
            i = Decibels::gainToDecibels(i, -200.0f) * 0.5;
            i = lerp(i, -100.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    InputBufferedHopBuffer<float> input_buffer;
    size_t complex_size;

    std::vector<float> window;
    std::vector<float> window_buffer;
    float normalisation_factor{1};

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

void BufferedSpectrogram::write(const float* begin, size_t num) {
    pimpl->write(begin, num);
}

BufferedSpectrogram::output_type BufferedSpectrogram::read_frame() {
    return pimpl->read_frame();
}

//----------------------------------------------------------------------------//

class BufferedMinMaxer::Impl final {
public:
    Impl(size_t buffer_size, size_t window_size, size_t hop_size)
            : input_buffer(buffer_size, window_size, hop_size, 0)
            , window_buffer(window_size) {
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
    void write(It begin, size_t num) {
        input_buffer.write(begin, num);
    }

    std::pair<float, float> read_frame() {
        input_buffer.read(window_buffer.data(), window_buffer.size());
        auto it =
                std::minmax_element(window_buffer.begin(), window_buffer.end());
        return std::make_pair(*it.first, *it.second);
    }

private:
    InputBufferedHopBuffer<float> input_buffer;
    std::vector<float> window_buffer;
};

//----------------------------------------------------------------------------//

BufferedMinMaxer::BufferedMinMaxer(size_t buffer_size,
                                   size_t window_size,
                                   size_t hop_size)
        : pimpl(std::make_unique<Impl>(buffer_size, window_size, hop_size)) {
}

BufferedMinMaxer::BufferedMinMaxer(BufferedMinMaxer&& rhs) noexcept = default;
BufferedMinMaxer& BufferedMinMaxer::operator=(BufferedMinMaxer&& rhs) noexcept =
        default;

BufferedMinMaxer::~BufferedMinMaxer() noexcept = default;

bool BufferedMinMaxer::has_waiting_frames() const {
    return pimpl->has_waiting_frames();
}

size_t BufferedMinMaxer::get_window_size() const {
    return pimpl->get_window_size();
}

size_t BufferedMinMaxer::get_hop_size() const {
    return pimpl->get_hop_size();
}

void BufferedMinMaxer::write(const input_type* begin, size_t num) {
    pimpl->write(begin, num);
}

BufferedMinMaxer::output_type BufferedMinMaxer::read_frame() {
    return pimpl->read_frame();
}