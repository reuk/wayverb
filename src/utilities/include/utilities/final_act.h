#pragma once

namespace util {

template <typename T>
class final_act final {
public:
    constexpr final_act() = default;
    constexpr final_act(T t)
            : t_{std::move(t)} {}

    ~final_act() noexcept {
        if (invoke_) {
            t_();
        }
    }

    void swap(final_act& other) noexcept {
        using std::swap;
        swap(t_, other.t_);
        swap(invoke_, other.invoke_);
    }

    final_act(const final_act&) = delete;
    constexpr final_act(final_act&& other) noexcept
            : t_{other.t_}
            , invoke_{other.invoke_} {
        other.invoke_ = false;
    }

    final_act& operator=(const final_act&) = delete;
    constexpr final_act& operator=(final_act&& other) noexcept {
        swap(other);
        return *this;
    }

private:
    T t_;
    bool invoke_{false};
};

template <typename T>
constexpr auto make_final_act(T t) {
    return final_act<T>{std::move(t)};
}

}  // namespace util
