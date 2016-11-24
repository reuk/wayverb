#pragma once

/// \file poly.h
/// Simple runtime polymorphism support for renderers and processors.

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"
#include "core/attenuator/null.h"

#include "utilities/aligned/vector.h"
#include "utilities/named_value.h"

struct processor {
    processor() = default;
    processor(const processor&) = default;
    processor(processor&&) noexcept = default;
    processor& operator=(const processor&) = default;
    processor& operator=(processor&&) noexcept = default;
    virtual ~processor() = default;

    virtual util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::null& attenuator) const = 0;
    virtual util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::hrtf& attenuator) const = 0;
    virtual util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::microphone& attenuator) const = 0;
};

template <typename Callback>
class concrete_processor final : public processor {
public:
    explicit concrete_processor(Callback callback)
            : callback_{std::move(callback)} {}

    util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::null& attenuator) const override {
        return callback_(attenuator);
    }
    util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::hrtf& attenuator) const override {
        return callback_(attenuator);
    }
    util::named_value<util::aligned::vector<float>> process(
            const wayverb::core::attenuator::microphone& attenuator)
            const override {
        return callback_(attenuator);
    }

private:
    Callback callback_;
};

template <typename Callback>
auto make_concrete_processor_ptr(Callback callback) {
    return std::make_unique<concrete_processor<Callback>>(std::move(callback));
}

////////////////////////////////////////////////////////////////////////////////

struct renderer {
    renderer() = default;
    renderer(const renderer&) = default;
    renderer(renderer&&) noexcept = default;
    renderer& operator=(const renderer&) = default;
    renderer& operator=(renderer&&) noexcept = default;
    virtual ~renderer() noexcept = default;

    virtual std::unique_ptr<processor> render() const = 0;
};

template <typename Callback>
class concrete_renderer final : public renderer {
public:
    explicit concrete_renderer(Callback callback)
            : callback_{std::move(callback)} {}

    std::unique_ptr<processor> render() const override {
        return make_concrete_processor_ptr(callback_());
    }

private:
    Callback callback_;
};

template <typename Callback>
auto make_concrete_renderer_ptr(Callback callback) {
    return std::make_unique<concrete_renderer<Callback>>(std::move(callback));
}
