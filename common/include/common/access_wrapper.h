#pragma once

#include <experimental/optional>

/// A class which holds a value and calls a callback before each access to the
/// value.
template <typename T>
class access_wrapper {
public:
    access_wrapper()                                        = default;
    access_wrapper(const access_wrapper&)                   = default;
    access_wrapper(access_wrapper&&) noexcept               = default;

    constexpr access_wrapper(const T& t)    : t_(t) {}
    constexpr access_wrapper(T&& t)         : t_(std::move(t)) {}

    template <typename... args>
    constexpr explicit access_wrapper(std::experimental::in_place_t,
                                      args&&... u)
            : t_(std::forward<args>(u)...) {}

    template <typename U,
              typename... args,
              typename std::enable_if_t<
                      std::is_constructible<T,
                                            std::initializer_list<U>&,
                                            args&&...>::value,
                      int> = 0>
    constexpr explicit access_wrapper(std::experimental::in_place_t,
                                      std::initializer_list<U> ilist,
                                      args&&... u)
            : t_(ilist, std::forward<args>(u)...) {}

    access_wrapper& operator=(const access_wrapper&)        = default;
    access_wrapper& operator=(access_wrapper&&) noexcept(
            std::is_nothrow_move_assignable<T>::value&&
                    std::is_nothrow_move_constructible<T>::value) = default;

    template <typename U>
    access_wrapper& operator=(U&& u) {
        t_ = std::forward<U>(u);
        return *this;
    }

    void swap(access_wrapper& other) noexcept {
        using std::swap;
        swap(t_, other.t_);
    }

    constexpr T& value() &                  { callback(); return t_; }
    constexpr const T& value() const&       { callback(); return t_; }
    constexpr T&& value() &&                { callback(); return t_; }
    constexpr const T&& value() const&&     { callback(); return t_; }

    constexpr T* operator->()               { callback(); return &t_; }
    constexpr const T* operator->() const   { callback(); return &t_; }

    constexpr T& operator*() &              { callback(); return t_; }
    constexpr const T& operator*() const&   { callback(); return t_; }
    constexpr T&& operator*() &&            { callback(); return t_; }
    constexpr const T&& operator*() const&& { callback(); return t_; }

protected:
    ~access_wrapper() noexcept = default;   //  disable polymorphic base usage

private:
    //  customisation point
	virtual void callback() = 0;

    T t_;
};
