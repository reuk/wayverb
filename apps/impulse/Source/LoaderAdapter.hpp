#pragma once

#include "MultichannelAdapter.hpp"
#include "for_each.h"

template <typename T>
class LoaderAdapter final
        : public AudioFormatWriter::ThreadedWriter::IncomingDataReceiver {
public:
    using input_type = typename T::input_type;
    using output_type = typename T::output_type;

    LoaderAdapter(std::unique_ptr<AudioFormatReader>&& reader,
                  size_t buffer_size,
                  size_t window_size,
                  size_t hop_size)
            : load_context(*this, std::move(reader))
            , buffer(load_context.get_num_channels(),
                     buffer_size,
                     window_size,
                     hop_size)
            , data(load_context.get_num_channels())
            , x_spacing(buffer.get_hop_size() /
                        load_context.get_sample_rate()) {
        load_context.start();
    }

    void reset(int num_channels, double sample_rate, int64 total_samples) {
        std::lock_guard<std::mutex> lck(mut);
        data.clear();
        data.resize(num_channels);
    }

    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) {
        std::lock_guard<std::mutex> lck(mut);
        buffer.write(new_data.getArrayOfReadPointers(), num_samples);
        while (buffer.has_waiting_frames()) {
            auto frame = buffer.read_frame();
            for_each([](auto& i, auto&& j) { i.push_back(std::move(j)); },
                     data.begin(),
                     data.end(),
                     frame.begin());
        }
    }

    float get_x_spacing() const {
        std::lock_guard<std::mutex> lck(mut);
        return x_spacing;
    }

    bool is_fully_loaded() const {
        std::lock_guard<std::mutex> lck(mut);
        return !buffer.has_waiting_frames();
    }

    size_t get_num_channels() const {
        std::lock_guard<std::mutex> lck(mut);
        return load_context.get_num_channels();
    }

    size_t get_length_in_samples() const {
        std::lock_guard<std::mutex> lck(mut);
        return load_context.get_length_in_samples();
    }

    double get_length_in_seconds() const {
        std::lock_guard<std::mutex> lck(mut);
        return load_context.get_length_in_seconds();
    }

    double get_sample_rate() const {
        std::lock_guard<std::mutex> lck(mut);
        return load_context.get_sample_rate();
    }

    const std::vector<std::vector<output_type>> get_data() const {
        std::lock_guard<std::mutex> lck(mut);
        return data;
    }

    const std::vector<output_type> get_channel(size_t channel) const {
        std::lock_guard<std::mutex> lck(mut);
        return data[channel];
    }

private:
    mutable std::mutex mut;
    LoadContext load_context;
    MultichannelAdapter<T> buffer;
    std::vector<std::vector<output_type>> data;
    float x_spacing;
};