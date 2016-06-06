#pragma once

#include "RenderHelpers.hpp"
#include "WorkQueue.hpp"

class ImpulseRenderer : public BaseRenderer {
public:
    enum class Mode { waveform, waterfall };

    ImpulseRenderer();
    virtual ~ImpulseRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

    void set_mode(Mode mode);

private:
    virtual BaseContextLifetime* get_context_lifetime() override;

    class ContextLifetime;
    std::unique_ptr<ContextLifetime> context_lifetime;

    mutable std::mutex mut;
};

//----------------------------------------------------------------------------//

class ImpulseRendererComponent : public BaseRendererComponent<ImpulseRenderer> {
public:
    ImpulseRendererComponent();

    void set_mode(ImpulseRenderer::Mode mode);
};