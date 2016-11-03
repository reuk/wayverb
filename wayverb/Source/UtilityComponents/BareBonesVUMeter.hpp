#pragma once

#include "VUMeter.hpp"

class BareBonesVUMeter : public juce::Component,
                         public BufferReader,
                         public DualVUMeter::Listener {
public:
    enum class Orientation { horizontal, vertical };
    BareBonesVUMeter(size_t channel, Orientation orientation);

    void paint(juce::Graphics& g) override;

    float get_abs_level() const;
    void set_abs_level(float l);

    float get_rms_level() const;
    void set_rms_level(float l);

    void reset();

    void vu_meter_levels_changed(DualVUMeter*, float abs, float rms) override;

private:
    void do_push_buffer(const float** channel_data,
                        int num_channels,
                        int num_samples) override;

    DualVUMeter vu_meter;
    const Orientation orientation;
};
