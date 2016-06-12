#pragma once

#include "GLAudioThumbnail.h"
#include "RenderHelpers.hpp"
#include "WorkQueue.hpp"

#include "Ruler.hpp"

class ImpulseRenderer : public BaseRenderer,
                        public GLAudioThumbnailBase,
                        public Ruler::Listener {
public:
    enum class Mode { waveform, waterfall };

    ImpulseRenderer(const AudioTransportSource& audio_transport_source);
    virtual ~ImpulseRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

    void set_mode(Mode mode);

    void ruler_visible_range_changed(Ruler* r,
                                     const Range<float>& range) override;

    //  inherited
    void clear() override;
    void load_from(AudioFormatManager&, const File& file) override;
    void reset(int num_channels,
               double sample_rate,
               int64 total_samples) override;
    void addBlock(int64 sample_number_in_source,
                  const AudioSampleBuffer& new_data,
                  int start_offset,
                  int num_samples) override;

    void set_amplitude_scale(float f);
    void set_time_scale(float f);
    void set_visible_range(const Range<float>& range);

private:
    void set_visible_range_impl(const Range<float>& range);

    virtual BaseContextLifetime* get_context_lifetime() override;

    const AudioTransportSource& audio_transport_source;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;

    mutable std::mutex mut;
};

//----------------------------------------------------------------------------//

class ImpulseRendererComponent : public BaseRendererComponent<ImpulseRenderer> {
public:
    using BaseRendererComponent<ImpulseRenderer>::BaseRendererComponent;
};