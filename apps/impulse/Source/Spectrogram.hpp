#pragma once

#include <memory>
#include <vector>

/// Responsible for buffering streamed audio and creating spectrogram 'slices'
class BufferedSpectrogram {
public:
    using input_type = float;
    using output_type = std::vector<float>;

    BufferedSpectrogram(size_t buffer_size,
                        size_t window_size,
                        size_t hop_size);

    BufferedSpectrogram(BufferedSpectrogram&& rhs) noexcept;
    BufferedSpectrogram& operator=(BufferedSpectrogram&& rhs) noexcept;

    virtual ~BufferedSpectrogram() noexcept;

    bool has_waiting_frames() const;
    size_t get_window_size() const;
    size_t get_hop_size() const;
    void write(const input_type* begin, size_t num);
    output_type read_frame();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

/// Responsible for buffering streamed audio and creating minmax chunks
class BufferedMinMaxer {
public:
    using input_type = float;
    using output_type = std::pair<input_type, input_type>;

    BufferedMinMaxer(size_t buffer_size, size_t window_size, size_t hop_size);

    BufferedMinMaxer(BufferedMinMaxer&& rhs) noexcept;
    BufferedMinMaxer& operator=(BufferedMinMaxer&& rhs) noexcept;

    virtual ~BufferedMinMaxer() noexcept;

    bool has_waiting_frames() const;
    size_t get_window_size() const;
    size_t get_hop_size() const;
    void write(const input_type* begin, size_t num);
    output_type read_frame();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};