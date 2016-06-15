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

void ConvolutionAudioSource::IrMap::set_ir(size_t in,
                                           size_t out,
                                           std::vector<float>&& t) {
    data[ChannelPair{in, out}] = std::forward<std::vector<float>>(t);
}

namespace {
/// This is gonna be freakish slow, but I can change it if it's a problem
template <typename T, typename F>
auto get_unique(T&& data, F&& f) {
    std::set<size_t> ret;
    for (const auto& i : data) {
        ret.insert(f(i));
    }
    return ret;
}
}  // namespace

size_t ConvolutionAudioSource::IrMap::get_num_inputs() const {
    return get_unique(data, [](const auto& i) { return i.first.in; }).size();
}

size_t ConvolutionAudioSource::IrMap::get_num_outputs() const {
    return get_unique(data, [](const auto& i) { return i.first.out; }).size();
}

bool ConvolutionAudioSource::IrMap::ChannelPair::operator<(
        const ChannelPair& rhs) const {
    return std::tie(in, out) < std::tie(rhs.in, rhs.out);
}

//----------------------------------------------------------------------------//

ConvolutionAudioSource::ConvolutionAudioSource(
        std::unique_ptr<AudioSource>&& input)
        : input(std::move(input)) {
}

ConvolutionAudioSource::~ConvolutionAudioSource() noexcept = default;

void ConvolutionAudioSource::prepareToPlay(int samplesPerBlockExpected,
                                           double sampleRate) {
    auto ins = ir_map.get_num_inputs();
    auto outs = ir_map.get_num_outputs();
    //  construct a new engine
    pimpl = std::make_unique<Impl>(ins, outs, Impl::kLatencyZero);
    //  copy local IRs to engine
    for (const auto& i : ir_map.data) {
        pimpl->set(i.first.in,
                   i.first.out,
                   i.second.data(),
                   i.second.size(),
                   true);
    }
}

void ConvolutionAudioSource::releaseResources() {
    pimpl = nullptr;
}

void ConvolutionAudioSource::getNextAudioBlock(
        const AudioSourceChannelInfo& buffer) {
    buffer.clearActiveBufferRegion();

    if (active) {
        assert(pimpl);
        auto scratch = buffer;
        input->getNextAudioBlock(scratch);
        pimpl->process(scratch.buffer->getArrayOfWritePointers(),
                       buffer.buffer->getArrayOfWritePointers(),
                       scratch.buffer->getNumChannels(),
                       buffer.buffer->getNumChannels(),
                       scratch.numSamples);
    }
}

void ConvolutionAudioSource::set_active(bool a) {
    active = a;
}
bool ConvolutionAudioSource::get_active() const {
    return active;
}

void ConvolutionAudioSource::set_ir(size_t in,
                                    size_t out,
                                    std::vector<float>&& t) {
    ir_map.set_ir(in, out, std::forward<std::vector<float>>(t));
}