#pragma once

#include <experimental/utility>
#include <memory>

namespace wayverb {
namespace combined {
namespace model {

/// A shared-pointer with value semantics.
/// Useful for use with `weak_ptr`, checking for ended lifetimes etc.
template <typename T>
class shared_value final {
public:
    /// We don't allow construction from a shared_ptr because that would allow
    /// storing nullptr.

    shared_value()
            : value_{std::make_shared<T>()} {}

    explicit shared_value(T t)
            : value_{std::make_shared<T>(std::move(t))} {}

    shared_value(const shared_value& other)
            : value_{std::make_shared<T>(*other.value_)} {}

    shared_value(shared_value&& other) noexcept
            : value_{std::make_shared<T>(std::move(*other.value_))} {}

    //  From shared value
    shared_value& operator=(const shared_value& other) {
        *value_ = *other.value_;
        return *this;
    }

    shared_value& operator=(shared_value&& other) noexcept {
        *value_ = std::move(*other.value_);
        return *this;
    }

    //  From stack value
    shared_value& operator=(T other) noexcept {
        *value_ = std::move(other);
        return *this;
    }

    T* get() const { return value_.get(); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    std::shared_ptr<T> get_shared_ptr() const { return value_; }
    std::weak_ptr<T> get_weak_ptr() const { return value_; }

    template <typename Archive>
    void load(Archive& archive) {
        archive(value_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(value_);
    }

private:
    std::shared_ptr<T> value_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
