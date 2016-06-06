#pragma once

#include "WorkQueue.hpp"

#include "modern_gl_utils/drawable.h"

#include "glm/glm.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class BaseContextLifetime : public ::Drawable {
public:
    void set_viewport(const glm::vec2& v);
    glm::vec2 get_viewport() const;
    float get_aspect() const;

private:
    glm::vec2 viewport;
};

//----------------------------------------------------------------------------//

class BaseRenderer : public OpenGLRenderer,
                     public ChangeBroadcaster,
                     public AsyncUpdater {
public:
    void set_viewport(const glm::vec2& u);

    template <typename T>
    void push_incoming(T&& t) {
        incoming_work_queue.push(std::forward<T>(t));
    }

    template <typename T>
    void push_outgoing(T&& t) {
        push_outgoing_impl(std::forward<T>(t));
    }

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

protected:
    virtual BaseContextLifetime* get_context_lifetime() = 0;

private:
    void handleAsyncUpdate() override;

    template <typename T>
    void push_outgoing_impl(T&& t) {
        outgoing_work_queue.push(std::forward<T>(t));
        triggerAsyncUpdate();
    }

    WorkQueue incoming_work_queue;
    WorkQueue outgoing_work_queue;
};

//----------------------------------------------------------------------------//

template <typename Renderer>
class BaseRendererComponent : public Component {
public:
    template <typename... Ts>
    BaseRendererComponent(Ts&&... ts)
            : renderer(std::forward<Ts>(ts)...) {
        open_gl_context.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
        open_gl_context.setRenderer(&renderer);
        open_gl_context.setContinuousRepainting(true);
        open_gl_context.attachTo(*this);
    }

    virtual ~BaseRendererComponent() noexcept {
        open_gl_context.detach();
    }

    void resized() override {
        renderer.set_viewport(glm::vec2{getWidth(), getHeight()});
    }

protected:
    OpenGLContext open_gl_context;
    Renderer renderer;
};