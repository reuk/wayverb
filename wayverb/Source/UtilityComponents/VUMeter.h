#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <numeric>

class BufferReader {
public:
    virtual ~BufferReader() noexcept = default;

    void push_buffer(const juce::AudioSourceChannelInfo &buffer);
    void push_buffer(const juce::AudioSampleBuffer &buffer);
    void push_buffer(const float **channel_data,
                     int num_channels,
                     int num_samples);

private:
    virtual void do_push_buffer(const float **channel_data,
                                int num_channels,
                                int num_samples) = 0;
};

//----------------------------------------------------------------------------//

class Meter : public BufferReader {
public:
    template <typename T>
    Meter(T &&strategy, size_t channel)
            : strategy(std::make_unique<TemplateMeterStrategy<T>>(
                      std::forward<T>(strategy)))
            , channel(channel) {}

    void reset();

    void update();

    float get_level() const;
    void set_level(float f);

private:
    void do_push_buffer(const float **channel_data,
                        int num_channels,
                        int num_samples) override;

    struct MeterStrategy {
        virtual ~MeterStrategy() noexcept = default;
        virtual float operator()(const float *channel_data,
                                 int num_samples) const = 0;
    };

    template <typename T>
    struct TemplateMeterStrategy : public MeterStrategy {
        TemplateMeterStrategy(T &&t)
                : t(std::forward<T>(t)) {}
        float operator()(const float *channel_data,
                         int num_samples) const override {
            return t(channel_data, num_samples);
        }

    private:
        T t;
    };

    std::unique_ptr<MeterStrategy> strategy;
    size_t channel;
    float target{0};
    float actual{0};
};

//----------------------------------------------------------------------------//

class VUMeter : public BufferReader, private juce::Timer {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener &) = default;
        Listener &operator=(const Listener &) = default;
        Listener(Listener &&) noexcept = default;
        Listener &operator=(Listener &&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void vu_meter_level_changed(VUMeter *, float level) = 0;
    };

    VUMeter(size_t channel);

    float get_level() const;
    void set_level(float l);

    void reset();

    void addListener(Listener *l);
    void removeListener(Listener *l);

private:
    void on_change();

    void do_push_buffer(const float **channel_data,
                        int num_channels,
                        int num_samples) override;

    void timerCallback() override;

    juce::ListenerList<Listener> listener_list;
    Meter meter;
};

class DualVUMeter : public BufferReader, private juce::Timer {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener &) = default;
        Listener &operator=(const Listener &) = default;
        Listener(Listener &&) noexcept = default;
        Listener &operator=(Listener &&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void vu_meter_levels_changed(DualVUMeter *,
                                             float abs,
                                             float rms) = 0;
    };

    DualVUMeter(size_t channel);

    float get_abs_level() const;
    void set_abs_level(float l);

    float get_rms_level() const;
    void set_rms_level(float l);

    void reset();

    void addListener(Listener *l);
    void removeListener(Listener *l);

private:
    void on_change();

    void do_push_buffer(const float **channel_data,
                        int num_channels,
                        int num_samples) override;

    void timerCallback() override;

    juce::ListenerList<Listener> listener_list;
    Meter abs_meter;
    Meter rms_meter;
};
