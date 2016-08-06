#pragma once

#include "WorkQueue.hpp"

#include "common/single_thread_access_checker.h"

#include "modern_gl_utils/drawable.h"
#include "modern_gl_utils/updatable.h"

#include "glm/glm.hpp"

#include "juce_opengl/juce_opengl.h"

#include <thread>

class BaseContextLifetime : public mglu::drawable, public mglu::updatable {
public:
    void set_viewport(const glm::vec2& v);
    glm::vec2 get_viewport() const;
    float get_aspect() const;

private:
    virtual void viewport_changed(const glm::vec2& v);

    glm::vec2 viewport;
};

//----------------------------------------------------------------------------//

template <typename ContextLifetime>
class RendererComponent final : public juce::Component {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void renderer_open_gl_context_created(
                const RendererComponent*) = 0;
        virtual void renderer_open_gl_context_closing(
                const RendererComponent*) = 0;

    protected:
        ~Listener() noexcept = default;
    };

    template <typename... Ts>
    RendererComponent(Ts&&... ts)
            : renderer(*this, std::forward<Ts>(ts)...) {
        open_gl_context.setOpenGLVersionRequired(
                juce::OpenGLContext::openGL3_2);
        open_gl_context.setRenderer(&renderer);
        open_gl_context.setComponentPaintingEnabled(false);
        open_gl_context.setContinuousRepainting(true);
        open_gl_context.setMultisamplingEnabled(true);
        open_gl_context.attachTo(*this);
    }

    virtual ~RendererComponent() noexcept { open_gl_context.detach(); }

    void resized() override {
        std::lock_guard<std::mutex> lck(mut);
        renderer.set_viewport(glm::vec2{getWidth(), getHeight()});
    }

    void mouseDown(const juce::MouseEvent& e) override {
        std::lock_guard<std::mutex> lck(mut);
        renderer.mouse_down(to_glm_vec2(e.getPosition()));
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        std::lock_guard<std::mutex> lck(mut);
        renderer.mouse_drag(to_glm_vec2(e.getPosition()));
    }

    void mouseUp(const juce::MouseEvent& e) override {
        std::lock_guard<std::mutex> lck(mut);
        renderer.mouse_up(to_glm_vec2(e.getPosition()));
    }

    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override {
        std::lock_guard<std::mutex> lck(mut);
        renderer.mouse_wheel_move(wheel.deltaY);
    }

    template <typename T>
    void context_command(T&& t) {
        std::lock_guard<std::mutex> lck(mut);
        renderer.context_command(std::forward<T>(t));
    }

    void addListener(Listener* l) {
        std::lock_guard<std::mutex> lck(mut);
        listener_list.add(l);
    }

    void removeListener(Listener* l) {
        std::lock_guard<std::mutex> lck(mut);
        listener_list.remove(l);
    }

private:
    //  called by the renderer
    void open_gl_context_created() {
        listener_list.call(&Listener::renderer_open_gl_context_created, this);
    }
    void open_gl_context_closing() {
        listener_list.call(&Listener::renderer_open_gl_context_closing, this);
    }

    //  I could combine this into the RendererComponent class, but then the
    //  created, render, closing methods would have to be public, which seems
    //  bad + unnecessary. Better to hide them away here in a private class.
    class Renderer final : public juce::OpenGLRenderer {
    public:
        using ContextLifetimeConstructor = std::function<ContextLifetime()>;

        explicit Renderer(RendererComponent& listener,
                          const ContextLifetimeConstructor& constructor)
                : listener(listener)
                , constructor(constructor) {}

        void newOpenGLContextCreated() override {
            context_lifetime = std::make_unique<lifetime_t>(constructor());
            outgoing_work_queue.push(
                    [this] { listener.open_gl_context_created(); });
        }

        void renderOpenGL() override {
            assert(context_lifetime);
            while (auto method = incoming_work_queue.pop()) {
                method(*context_lifetime);
            }
            context_lifetime->update(0);
            context_lifetime->draw(glm::mat4{});
        }

        void openGLContextClosing() override {
            outgoing_work_queue.push(
                    [this] { listener.open_gl_context_closing(); });
            context_lifetime = nullptr;
        }

        template <typename T>
        void context_command(T&& t) {
            incoming_work_queue.push(std::forward<T>(t));
        }

        void set_viewport(const glm::vec2& u) {
            incoming_work_queue.push([u](auto& i) { i.set_viewport(u); });
        }

        void mouse_down(const glm::vec2& u) {
            incoming_work_queue.push([u](auto& i) { i.mouse_down(u); });
        }

        void mouse_drag(const glm::vec2& u) {
            incoming_work_queue.push([u](auto& i) { i.mouse_drag(u); });
        }

        void mouse_up(const glm::vec2& u) {
            incoming_work_queue.push([u](auto& i) { i.mouse_up(u); });
        }

        void mouse_wheel_move(float u) {
            incoming_work_queue.push([u](auto& i) { i.mouse_wheel_move(u); });
        }

    private:
        WorkQueue<ContextLifetime&> incoming_work_queue;
        AsyncWorkQueue outgoing_work_queue;

        RendererComponent& listener;

        ContextLifetimeConstructor constructor;

        using lifetime_t = ContextLifetime;
        using lifetime_ptr = std::unique_ptr<lifetime_t>;
        lifetime_ptr context_lifetime;
    };

    template <typename T>
    static glm::vec2 to_glm_vec2(const T& t) {
        return glm::vec2{t.x, t.y};
    }

    mutable std::mutex mut;

    juce::OpenGLContext open_gl_context;
    Renderer renderer;

    juce::ListenerList<Listener> listener_list;
};