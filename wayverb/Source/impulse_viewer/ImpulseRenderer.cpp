#include "ImpulseRenderer.hpp"

class ImpulseRenderer::ContextLifetime : public BaseContextLifetime {
public:
private:
};

//----------------------------------------------------------------------------//

ImpulseRenderer::ImpulseRenderer() {
}

ImpulseRenderer::~ImpulseRenderer() noexcept = default;

void ImpulseRenderer::newOpenGLContextCreated() {
    BaseRenderer::newOpenGLContextCreated();
}

void ImpulseRenderer::openGLContextClosing() {
    BaseRenderer::openGLContextClosing();
}

BaseContextLifetime* ImpulseRenderer::get_context_lifetime() {
    return context_lifetime.get();
}

//----------------------------------------------------------------------------//

ImpulseRendererComponent::ImpulseRendererComponent() {
}