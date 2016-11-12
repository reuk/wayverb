#include "BareBonesVUMeter.h"
#include "GainSlider.h"

BareBonesVUMeter::BareBonesVUMeter(size_t channel, Orientation orientation)
        : vu_meter(channel)
        , orientation(orientation) {
    vu_meter.addListener(this);
}

void BareBonesVUMeter::paint(juce::Graphics& g) {
    const auto get_proportion = [](double g) {
        return gain_transform::db_gain_to_proportion(
                juce::Decibels::gainToDecibels(g));
    };

    const auto peaking_db = -0.0;
    const auto peaking_prop = gain_transform::db_gain_to_proportion(peaking_db);

    const auto abs_prop = get_proportion(vu_meter.get_abs_level());
    const auto rms_prop = get_proportion(vu_meter.get_rms_level());
    const auto base_colour =
            peaking_prop < abs_prop ? juce::Colours::red : juce::Colours::green;

    const auto get_bounds = [this](auto prop) {
        switch (orientation) {
            case Orientation::horizontal:
                return getLocalBounds().withWidth(getWidth() * prop);
            case Orientation::vertical:
                return getLocalBounds().withTrimmedTop(getHeight() *
                                                       (1.0 - prop));
        }
    };

    const auto abs_bounds = get_bounds(abs_prop);
    const auto rms_bounds = get_bounds(rms_prop);

    g.setColour(base_colour);
    g.fillRect(abs_bounds);
    g.setColour(base_colour.darker());
    g.fillRect(rms_bounds);
}

float BareBonesVUMeter::get_abs_level() const {
    return vu_meter.get_abs_level();
}
void BareBonesVUMeter::set_abs_level(float l) {
    vu_meter.set_abs_level(l);
}

float BareBonesVUMeter::get_rms_level() const {
    return vu_meter.get_rms_level();
}
void BareBonesVUMeter::set_rms_level(float l) {
    vu_meter.set_rms_level(l);
}

void BareBonesVUMeter::reset() {
    vu_meter.reset();
}

void BareBonesVUMeter::vu_meter_levels_changed(DualVUMeter*, float, float) {
    repaint();
}

void BareBonesVUMeter::do_push_buffer(const float** channel_data,
                                      int num_channels,
                                      int num_samples) {
    vu_meter.push_buffer(channel_data, num_channels, num_samples);
}
