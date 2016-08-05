#pragma once

class callback_base {
public:
    callback_base(callback_base&&) noexcept = default;
    callback_base& operator=(callback_base&&) noexcept = default;
    virtual ~callback_base() noexcept = default;

    virtual void method_one();
    virtual void method_two();
};

template <typename T>
class callback_impl : public callback_base {
public:
    callback_impulse(T& t)
            : t(&t) {}

    void method_one() override { t->method_one(); }
    void method_two() override { t->method_two(); }

private:
    T& t;
};

template <typename... Ts>
class observer_collection final {
public:
    template <typename T>
    void add_callback(T&& t) {
        callbacks.push_back(std::forward<T>(t));
    }

    void call(Ts&&... ts) {
        for (auto& i : callbacks) {
            i(std::forward<Ts>(ts)...);
        }
    }

private:
    aligned::vector<std::function<void(Ts...)>> callbacks;
};
