#pragma once

#include "threaded_queue.h"
#include "async_work_queue.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

template <typename Renderer>
class generic_renderer final : public Component {
    /// This class will be used entirely from the gl thread (other than
    /// construction/destruction).
    /// It communicates with the outside world using thread-safe queues.
    class impl final : public OpenGLRenderer {
    public:
        using input_queue = threaded_queue<std::function<void(Renderer&)>>;

        impl(generic_renderer& owner)
                : owner_{owner} {}

        void high_priority_command(typename input_queue::value_type command) {
            high_priority_queue_.push(std::move(command));
        }

        void low_priority_command(typename input_queue::value_type command) {
            low_priority_queue_.push(std::move(command));
        }

    private:
        void newOpenGLContextCreated() override {
            //  Create a new renderer object.
            renderer_ = std::make_unique<Renderer>();
            //  Signal that the context was created.
            output_queue_.push([this] { owner_.context_created(); });
        }

        void renderOpenGL() override {
            //  Do all pending high-priority actions
            while (auto method = high_priority_queue_.pop()) {
                (*method)(*renderer_);
            }

            //  Do some of the low-priority actions, but bin any we don't have
            //  time for.
            const auto start = std::chrono::system_clock::now();
            while (auto method = low_priority_queue_.pop()) {
                (*method)(*renderer_);
                if (std::chrono::milliseconds{16} <
                    std::chrono::system_clock::now() - start) {
                    low_priority_queue_.clear();
                    break;
                }
            }

            //  Update and draw.
            renderer_->update(0);
            renderer_->draw(glm::mat4{});
        }

        void openGLContextClosing() override {
            //  Signal that the context is closing.
            output_queue_.push([this] { owner_.context_closing(); });
            //  Delete renderer object.
            renderer_ = nullptr;
        }

        generic_renderer& owner_;

        std::unique_ptr<Renderer> renderer_;

        input_queue high_priority_queue_;
        input_queue low_priority_queue_;
        async_work_queue output_queue_;
    };

public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(const Listener&) = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void context_created(generic_renderer&) = 0;
        virtual void context_closing(generic_renderer&) = 0;
    };

    generic_renderer()
            : impl_{*this} {
        context_.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
        context_.setRenderer(&impl_);
        context_.setComponentPaintingEnabled(false);
        context_.setContinuousRepainting(true);
        context_.setMultisamplingEnabled(true);
        context_.attachTo(*this);
    }

    ~generic_renderer() noexcept { context_.detach(); }

    void high_priority_command(typename impl::input_queue::value_type command) {
        impl_.high_priority_command(std::move(command));
    }

    void low_priority_command(typename impl::input_queue::value_type command) {
        impl_.low_priority_command(std::move(command));
    }

    void addListener(Listener* l) { listener_list_.add(l); }

    void removeListener(Listener* l) { listener_list_.remove(l); }

private:
    void context_created() {
        listener_list_.call(&Listener::context_created, *this);
    }

    void context_closing() {
        listener_list_.call(&Listener::context_closing, *this);
    }

    OpenGLContext context_;
    impl impl_;

    ListenerList<Listener> listener_list_;
};
