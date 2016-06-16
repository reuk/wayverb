#include "VUMeter.hpp"
#include "VisualiserLookAndFeel.hpp"

#include <numeric>

void BufferReader::push_buffer(const AudioSourceChannelInfo& buffer) {
    push_buffer(*buffer.buffer);
}
void BufferReader::push_buffer(const AudioSampleBuffer& buffer) {
    push_buffer(buffer.getArrayOfReadPointers(),
                buffer.getNumChannels(),
                buffer.getNumSamples());
}

void Meter::paint(Graphics& g) {
    g.setColour(VisualiserLookAndFeel::emphasis);
    g.fillRect(getLocalBounds().withWidth(getWidth() * actual));
}

void Meter::push_buffer(const float** channel_data,
                        int num_channels,
                        int num_samples) {
    target = channel < num_channels
                     ? (*strategy)(channel_data[channel], num_samples)
                     : 0;
    actual = std::max(target, actual);
    actual += (target - actual) * 0.1;
}

void Meter::reset() {
    target = actual = 0;
}

//----------------------------------------------------------------------------//

VUMeter::VUMeter(int channel)
        : abs_meter(channel,
                    VisualiserLookAndFeel::emphasis.brighter(),
                    [](const float* channel_data, int num_samples) {
                        return std::accumulate(channel_data,
                                               channel_data + num_samples,
                                               0.0f,
                                               [](auto i, auto j) {
                                                   return std::max(i,
                                                                   std::abs(j));
                                               });
                    })
        , rms_meter(channel,
                    VisualiserLookAndFeel::emphasis.darker(),
                    [](const float* channel_data, int num_samples) {
                        return std::sqrt(
                                std::accumulate(channel_data,
                                                channel_data + num_samples,
                                                0.0f,
                                                [](auto i, auto j) {
                                                    return i + j * j;
                                                }) /
                                num_samples);
                    }) {
    addAndMakeVisible(abs_meter);
    addAndMakeVisible(rms_meter);

    abs_meter.toBehind(&rms_meter);

    startTimerHz(60);
}

void VUMeter::paint(Graphics& g) {
    g.fillAll(Colours::lightgrey);
}

void VUMeter::resized() {
    abs_meter.setBounds(getLocalBounds());
    rms_meter.setBounds(getLocalBounds());
}

void VUMeter::push_buffer(const float** channel_data,
                          int num_channels,
                          int num_samples) {
    abs_meter.push_buffer(channel_data, num_channels, num_samples);
    rms_meter.push_buffer(channel_data, num_channels, num_samples);
}

void VUMeter::reset() {
    abs_meter.reset();
    rms_meter.reset();
}

void VUMeter::timerCallback() {
    repaint();
}