#pragma once

template <typename... Ts>
struct GenericArgumentsCallback {
    virtual ~GenericArgumentsCallback() noexcept = default;

    virtual void operator()(Ts&&... ts) const {
    }
};

template <>
struct GenericArgumentsCallback<void> {
    virtual ~GenericArgumentsCallback() noexcept = default;

    virtual void operator()() const {
    }
};

template <typename Callback, typename... Ts>
struct GenericCallbackAdapter : public GenericArgumentsCallback<Ts...> {
    explicit GenericCallbackAdapter(const Callback& c = Callback())
            : c(c) {
    }

    void operator()(Ts&&... ts) const override {
        c(std::forward<Ts>(ts)...);
    }

private:
    const Callback& c;
};

template <typename Callback>
struct GenericCallbackAdapter<Callback, void>
        : public GenericArgumentsCallback<void> {
    explicit GenericCallbackAdapter(const Callback& c = Callback())
            : c(c) {
    }

    void operator()() const override {
        c();
    }

private:
    const Callback& c;
};

//----------------------------------------------------------------------------//

using DoNothingCallback = GenericArgumentsCallback<void>;

template <typename Callback>
auto make_callback(const Callback& c) {
    return GenericCallbackAdapter<Callback, void>(c);
}
