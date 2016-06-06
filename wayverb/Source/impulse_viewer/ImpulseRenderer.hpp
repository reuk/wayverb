#pragma once

#include "RenderHelpers.hpp"
#include "WorkQueue.hpp"

class ImpulseRenderer : public BaseRenderer {
public:
    ImpulseRenderer();
    virtual ~ImpulseRenderer() noexcept;

    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;

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

private:
};