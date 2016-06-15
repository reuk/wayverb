#pragma once

#include "ConvolutionRoutingComponent.hpp"

#include <map>

class ConvolutionAudioSource : public AudioSource {
public:
    ConvolutionAudioSource(AudioSource* source, bool handle_delete);
    virtual ~ConvolutionAudioSource() noexcept;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& buffer) override;

    void set_active(bool a);
    bool get_active() const;

    void set_ir(size_t output_channel, const std::vector<float>& t);

    void set_carrier_routing(const std::vector<CarrierRouting>& c);

private:
    void set_engine_ir();

    OptionalScopedPointer<AudioSource> source;
    std::atomic_bool active{true};

    std::vector<std::vector<float>> impulses;
    std::vector<CarrierRouting> carrier_routing;

    /// This will be initialised on prepareToPlay and destroyed on release
    class Impl;
    std::unique_ptr<Impl> pimpl;
};