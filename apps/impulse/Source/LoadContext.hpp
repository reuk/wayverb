#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class LoadContext {
public:
    LoadContext(AudioFormatWriter::ThreadedWriter::IncomingDataReceiver& o,
                std::unique_ptr<AudioFormatReader>&& m);
    virtual ~LoadContext() noexcept;

    void start();

    bool is_fully_loaded() const;

    size_t get_num_channels() const;
    size_t get_length_in_samples() const;
    double get_length_in_seconds() const;
    double get_sample_rate() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};