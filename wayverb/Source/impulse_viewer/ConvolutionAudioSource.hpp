#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <map>

class ConvolutionAudioSource : public AudioSource {
public:
    ConvolutionAudioSource(AudioSource * source, bool handle_delete);
    virtual ~ConvolutionAudioSource() noexcept;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& buffer) override;

    void set_active(bool a);
    bool get_active() const;

    void set_ir(size_t in, size_t out, std::vector<float>&& t);

private:
    OptionalScopedPointer<AudioSource> source;
    std::atomic_bool active{true};

    /// Manages the mapping between inputs, outputs, and IRs
    class IrMap {
    public:
        void set_ir(size_t in, size_t out, std::vector<float>&& t);

        size_t get_num_inputs() const;
        size_t get_num_outputs() const;

        struct ChannelPair {
            size_t in;
            size_t out;

            bool operator<(const ChannelPair& rhs) const;
        };

        std::map<ChannelPair, std::vector<float>> data;
    };
    IrMap ir_map;

    /// This will be initialised on prepareToPlay and destroyed on release
    class Impl;
    std::unique_ptr<Impl> pimpl;
};