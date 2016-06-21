#pragma once

#include "RenderHelpers.hpp"
#include "WorkQueue.hpp"

#include "Ruler.hpp"

class ImpulseRenderer : public BaseRenderer {
public:
    enum class Mode { waveform, waterfall };

    ImpulseRenderer(const AudioTransportSource& audio_transport_source,
                    AudioFormatManager& manager,
                    const File& file);
    virtual ~ImpulseRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

    void set_mode(Mode mode);

    void set_visible_range(const Range<double>& range);

private:
    void set_visible_range_impl(const Range<double>& range);
    void set_mode_impl(Mode m);

    virtual BaseContextLifetime* get_context_lifetime() override;

    mutable std::mutex mut;

    const AudioTransportSource& audio_transport_source;
    AudioFormatManager& audio_format_manager;
    File file;

    Range<double> visible_range;
    Mode mode;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;
};

//----------------------------------------------------------------------------//

class ImpulseRendererComponent : public BaseRendererComponent<ImpulseRenderer> {
public:
    using BaseRendererComponent<ImpulseRenderer>::BaseRendererComponent;
};