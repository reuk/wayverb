#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class BufferReader {
public:
    virtual ~BufferReader() noexcept = default;

    void push_buffer(const AudioSourceChannelInfo& buffer);
    void push_buffer(const AudioSampleBuffer& buffer);
    virtual void push_buffer(const float** channel_data,
                             int num_channels,
                             int num_samples) = 0;
};

class Meter : public Component, public BufferReader {
public:
    template <typename T>
    Meter(int channel, const Colour& colour, T&& strategy)
            : channel(channel)
            , colour(colour)
            , strategy(std::make_unique<TemplateMeterStrategy<T>>(
                      std::forward<T>(strategy))) {
    }

    void paint(Graphics& g) override;

    void push_buffer(const float** channel_data,
                     int num_channels,
                     int num_samples) override;

    void reset();

private:
    struct MeterStrategy {
        virtual ~MeterStrategy() noexcept = default;
        virtual float operator()(const float* channel_data,
                                 int num_samples) const = 0;
    };

    template <typename T>
    struct TemplateMeterStrategy : public MeterStrategy {
        TemplateMeterStrategy(T&& t)
                : t(std::forward<T>(t)) {
        }
        float operator()(const float* channel_data,
                         int num_samples) const override {
            return t(channel_data, num_samples);
        }

    private:
        T t;
    };

    int channel;
    Colour colour;
    float target{0};
    float actual{0};
    std::unique_ptr<MeterStrategy> strategy;
};

//----------------------------------------------------------------------------//

class VUMeter : public Component, public BufferReader, private Timer {
public:
    explicit VUMeter(int channel);

    void paint(Graphics& g) override;
    void resized() override;

    void push_buffer(const float** channel_data,
                     int num_channels,
                     int num_samples) override;

    void reset();

private:
    void timerCallback() override;

    Meter abs_meter;
    Meter rms_meter;
};