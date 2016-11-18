#pragma once

#include "utilities/event.h"
#include "utilities/threaded_queue.h"

#include "async_work_queue.h"

#include "../JuceLibraryCode/JuceHeader.h"

#include "glm/glm.hpp"

/// Oooooooooooh this class is well good.
template <typename Renderer>
class generic_renderer final : public Component {
    /// This class will be used entirely from the gl thread (other than
    /// construction/destruction).
    /// It communicates with the outside world using thread-safe queues.
    class impl final : public OpenGLRenderer {
    public:
        using input_queue =
                util::threaded_queue<util::threading_policy::scoped_lock,
                                     std::function<void(Renderer&)>>;

        impl(input_queue& high_queue, input_queue& low_queue)
                : high_priority_queue_{high_queue}
                , low_priority_queue_{low_queue} {}

        using context_created = util::event<>;
        typename context_created::connection connect_context_created(
                typename context_created::callback_type callback) {
            return context_created_.connect(std::move(callback));
        }

        using context_closing = util::event<>;
        typename context_closing::connection connect_context_closing(
                typename context_closing::callback_type callback) {
            return context_closing_.connect(std::move(callback));
        }

    private:
        void newOpenGLContextCreated() override {
            //  Create a new renderer object.
            renderer_ = std::make_unique<Renderer>();
            //  Signal that the context was created.
            output_queue_.push([this] { context_created_(); });
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
            output_queue_.push([this] { context_closing_(); });
            //  Delete renderer object.
            renderer_ = nullptr;
        }

        std::unique_ptr<Renderer> renderer_;

        input_queue& high_priority_queue_;
        input_queue& low_priority_queue_;
        async_work_queue output_queue_;

        context_created context_created_;
        context_closing context_closing_;
    };

public:
    generic_renderer()
            : impl_{high_priority_queue_, low_priority_queue_} {
        impl_.connect_context_created([this] { context_created_(*this); });
        impl_.connect_context_closing([this] { context_closing_(*this); });

        context_.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
        context_.setRenderer(&impl_);
        context_.setComponentPaintingEnabled(false);
        context_.setContinuousRepainting(true);
        context_.setMultisamplingEnabled(true);
        context_.attachTo(*this);
    }

    ~generic_renderer() noexcept { context_.detach(); }

    void high_priority_command(typename impl::input_queue::value_type command) {
        high_priority_queue_.push(std::move(command));
    }

    void low_priority_command(typename impl::input_queue::value_type command) {
        low_priority_queue_.push(std::move(command));
    }

    using context_created = util::event<generic_renderer<Renderer>&>;
    typename context_created::connection connect_context_created(
            typename context_created::callback_type callback) {
        return context_created_.connect(std::move(callback));
    }

    using context_closing = util::event<generic_renderer<Renderer>&>;
    typename context_closing::connection connect_context_closing(
            typename context_closing::callback_type callback) {
        return context_closing_.connect(std::move(callback));
    }

private:
    typename impl::input_queue high_priority_queue_;
    typename impl::input_queue low_priority_queue_;

    OpenGLContext context_;
    impl impl_;

    context_created context_created_;
    context_closing context_closing_;
};
