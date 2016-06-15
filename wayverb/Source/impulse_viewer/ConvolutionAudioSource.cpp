#include "ConvolutionAudioSource.hpp"
#include <set>

namespace {

/// Mock convolver to stand in for the hiss convolver
class Convolver {
public:
    enum LatencyMode { kLatencyZero, kLatencyShort, kLatencyMedium };

    Convolver(uint32_t numIns, uint32_t numOuts, LatencyMode latency) {
        std::cout << "construct convolver" << std::endl;
    }
    Convolver(uint32_t numIO, LatencyMode latency) {
        std::cout << "construct convolver" << std::endl;
    }

    virtual ~Convolver() noexcept = default;

    void clear(bool resize) {
        std::cout << "clear" << std::endl;
    }
    void clear(uint32_t inChan, uint32_t outChan, bool resize) {
        std::cout << "clear" << std::endl;
    }

    void reset() {
        std::cout << "reset" << std::endl;
    }
    void reset(uint32_t inChan, uint32_t outChan) {
        std::cout << "reset" << std::endl;
    }

    void resize(uint32_t inChan, int32_t outChan, uintptr_t impulseLength) {
        std::cout << "resize" << std::endl;
    }

    void set(uint32_t inChan,
             int32_t outChan,
             const float* input,
             uintptr_t length,
             bool resize) {
        std::cout << "set" << std::endl;
    }

    void process(float** ins,
                 float** outs,
                 uint32_t numIns,
                 uint32_t numOuts,
                 uintptr_t numSamples) {
        //  no cout here because IO on the audio thread is a mortal sin
    }
};
}  // namespace

class ConvolutionAudioSource::Impl : public Convolver {
    using Convolver::Convolver;
};

//----------------------------------------------------------------------------//

ConvolutionAudioSource::ConvolutionAudioSource(AudioSource* source,
                                               bool handle_delete)
        : source(source, handle_delete) {
}
ConvolutionAudioSource::~ConvolutionAudioSource() noexcept = default;

void ConvolutionAudioSource::prepareToPlay(int samplesPerBlockExpected,
                                           double sampleRate) {
    source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    //  construct a new engine
    auto num_channels = impulses.size();
    pimpl = std::make_unique<Impl>(
            num_channels, num_channels, Impl::kLatencyZero);
    //  copy local IRs to engine
    set_engine_ir();
}

void ConvolutionAudioSource::set_engine_ir() {
    assert(pimpl);
    for (auto i = 0u; i != impulses.size(); ++i) {
        pimpl->set(i, i, impulses[i].data(), impulses[i].size(), true);
    }
}

void ConvolutionAudioSource::releaseResources() {
    pimpl = nullptr;
}

void ConvolutionAudioSource::getNextAudioBlock(
        const AudioSourceChannelInfo& buffer) {
    source->getNextAudioBlock(buffer);

    AudioSampleBuffer scratch;
    scratch.makeCopyOf(*buffer.buffer);
    scratch.clear(buffer.startSample, buffer.numSamples);

    //  for each carrier channel
    auto carrier_channels = std::min(static_cast<int>(carrier_routing.size()),
                                     buffer.buffer->getNumChannels());
    for (auto i = 0u; i != carrier_channels; ++i) {
        //  for each hardware channel
        auto hardware_channels =
                std::min(static_cast<int>(carrier_routing[i].channel.size()),
                         scratch.getNumChannels());
        for (auto j = 0u; j != hardware_channels; ++j) {
            if (carrier_routing[i].channel[j]) {
                scratch.addFrom(j,
                                buffer.startSample,
                                *buffer.buffer,
                                i,
                                buffer.startSample,
                                buffer.numSamples);
            }
        }
    }

    if (active) {
        assert(pimpl);
        buffer.clearActiveBufferRegion();
        pimpl->process(scratch.getArrayOfWritePointers(),
                       buffer.buffer->getArrayOfWritePointers(),
                       scratch.getNumChannels(),
                       buffer.buffer->getNumChannels(),
                       buffer.numSamples);
    } else {
        *buffer.buffer = scratch;
    }
}

void ConvolutionAudioSource::set_active(bool a) {
    active = a;
}
bool ConvolutionAudioSource::get_active() const {
    return active;
}

/// Won't have any effect if audio has already started
/// TODO maybe that's a bad idea tho
void ConvolutionAudioSource::set_ir(size_t output_channel,
                                    const std::vector<float>& t) {
    if (impulses.size() <= output_channel) {
        impulses.resize(output_channel + 1);
    }
    impulses[output_channel] = t;

    if (pimpl) {
        set_engine_ir();
    }
}

void ConvolutionAudioSource::set_carrier_routing(
        const std::vector<CarrierRouting>& c) {
    carrier_routing = c;
}