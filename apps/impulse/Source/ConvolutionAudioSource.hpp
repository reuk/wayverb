#pragma once

#include "Routing.h"

#include <map>

class ConvolutionAudioSource : public AudioSource {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void carrier_signal_in(ConvolutionAudioSource*,
                                       const float** input,
                                       int num_channels,
                                       int num_samples) = 0;
        virtual void impulse_signal_in(ConvolutionAudioSource*,
                                       const float** input,
                                       int num_channels,
                                       int num_samples) = 0;
    };

    ConvolutionAudioSource(AudioSource* source, bool handle_delete);
    virtual ~ConvolutionAudioSource() noexcept;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& buffer) override;

    void set_active(bool a);
    bool get_active() const;

    void set_ir(size_t output_channel, const std::vector<float>& t);

    void set_carrier_routing(const std::vector<model::CarrierRouting>& c);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    void set_engine_ir();

    OptionalScopedPointer<AudioSource> source;
    std::atomic_bool active{true};

    std::vector<std::vector<float>> impulses;
    std::vector<model::CarrierRouting> carrier_routing;

    /// This will be initialised on prepareToPlay and destroyed on release
    class Impl;
    std::unique_ptr<Impl> pimpl;

    ListenerList<Listener> listener_list;
};