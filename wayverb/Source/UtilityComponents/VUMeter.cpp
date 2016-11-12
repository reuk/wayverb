#include "VUMeter.h"

void BufferReader::push_buffer(const juce::AudioSourceChannelInfo &buffer) {
    push_buffer(*buffer.buffer);
}
void BufferReader::push_buffer(const juce::AudioSampleBuffer &buffer) {
    push_buffer(buffer.getArrayOfReadPointers(),
                buffer.getNumChannels(),
                buffer.getNumSamples());
}
void BufferReader::push_buffer(const float **channel_data,
                               int num_channels,
                               int num_samples) {
    do_push_buffer(channel_data, num_channels, num_samples);
}

//----------------------------------------------------------------------------//

void Meter::do_push_buffer(const float **channel_data,
                           int num_channels,
                           int num_samples) {
    set_level(channel < num_channels
                      ? (*strategy)(channel_data[channel], num_samples)
                      : 0);
}

void Meter::reset() {
    target = actual = 0;
}

void Meter::update() {
    actual += (target - actual) * 0.1;
}

float Meter::get_level() const {
    return actual;
}

void Meter::set_level(float f) {
    target = f;
    actual = std::max(target, actual);
}

//----------------------------------------------------------------------------//

namespace {
float abs_meter_strategy(const float *channel_data, int num_samples) {
    return std::accumulate(
            channel_data, channel_data + num_samples, 0.0f, [](auto i, auto j) {
                return std::max(i, std::abs(j));
            });
}
}  // namespace

//----------------------------------------------------------------------------//

VUMeter::VUMeter(size_t channel)
        : meter(abs_meter_strategy, channel) {
    startTimerHz(60);
}

float VUMeter::get_level() const {
    return meter.get_level();
}

void VUMeter::set_level(float l) {
    return meter.set_level(l);
}

void VUMeter::reset() {
    meter.reset();
}

void VUMeter::addListener(Listener *l) {
    listener_list.add(l);
}

void VUMeter::removeListener(Listener *l) {
    listener_list.remove(l);
}

void VUMeter::on_change() {
    listener_list.call(&Listener::vu_meter_level_changed, this, get_level());
}

void VUMeter::do_push_buffer(const float **channel_data,
                             int num_channels,
                             int num_samples) {
    meter.push_buffer(channel_data, num_channels, num_samples);
}

void VUMeter::timerCallback() {
    listener_list.call(&Listener::vu_meter_level_changed, this, get_level());
    meter.update();
}

//----------------------------------------------------------------------------//

DualVUMeter::DualVUMeter(size_t channel)
        : abs_meter(abs_meter_strategy, channel)
        , rms_meter(
                  [](const float *channel_data, int num_samples) {
                      return std::sqrt(
                              std::accumulate(channel_data,
                                              channel_data + num_samples,
                                              0.0f,
                                              [](auto i, auto j) {
                                                  return i + j * j;
                                              }) /
                              num_samples);
                  },
                  channel) {
    startTimerHz(60);
}

void DualVUMeter::on_change() {
    listener_list.call(&Listener::vu_meter_levels_changed,
                       this,
                       get_abs_level(),
                       get_rms_level());
}

void DualVUMeter::do_push_buffer(const float **channel_data,
                                 int num_channels,
                                 int num_samples) {
    abs_meter.push_buffer(channel_data, num_channels, num_samples);
    rms_meter.push_buffer(channel_data, num_channels, num_samples);
}

void DualVUMeter::reset() {
    abs_meter.reset();
    rms_meter.reset();
}

void DualVUMeter::timerCallback() {
    on_change();
    abs_meter.update();
    rms_meter.update();
}

float DualVUMeter::get_abs_level() const {
    return abs_meter.get_level();
}

float DualVUMeter::get_rms_level() const {
    return rms_meter.get_level();
}

void DualVUMeter::set_abs_level(float l) {
    abs_meter.set_level(l);
}

void DualVUMeter::set_rms_level(float l) {
    rms_meter.set_level(l);
}

void DualVUMeter::addListener(Listener *l) {
    listener_list.add(l);
}

void DualVUMeter::removeListener(Listener *l) {
    listener_list.remove(l);
}
