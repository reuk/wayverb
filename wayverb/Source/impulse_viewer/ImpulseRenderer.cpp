#include "ImpulseRenderer.hpp"

class ImpulseRenderer::ContextLifetime : public BaseContextLifetime {
public:
    ContextLifetime() = default;

    void draw() const override {
        auto c = 0.5;
        glClearColor(c, c, c, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        /*
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        */
    }
};

//----------------------------------------------------------------------------//

ImpulseRenderer::ImpulseRenderer() {
}

ImpulseRenderer::~ImpulseRenderer() noexcept = default;

void ImpulseRenderer::newOpenGLContextCreated() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = std::make_unique<ContextLifetime>();
    BaseRenderer::newOpenGLContextCreated();
}

void ImpulseRenderer::openGLContextClosing() {
    std::lock_guard<std::mutex> lck(mut);
    context_lifetime = nullptr;
    BaseRenderer::openGLContextClosing();
}

BaseContextLifetime* ImpulseRenderer::get_context_lifetime() {
    return context_lifetime.get();
}

//----------------------------------------------------------------------------//

ImpulseRendererComponent::ImpulseRendererComponent() {
}