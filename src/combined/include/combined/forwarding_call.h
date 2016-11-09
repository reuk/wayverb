#pragma once

namespace wayverb {
namespace combined {

template <typename T>
class forwarding_call final {
public:
    constexpr explicit forwarding_call(T& t)
            : t_{&t} {}

    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) const {
        (*t_)(std::forward<Ts>(ts)...);
    }

private:
    T* t_;
};

template <typename T>
constexpr auto make_forwarding_call(T& t) {
    return forwarding_call<T>{t};
}

}  // namespace combined
}  // namespace wayverb
