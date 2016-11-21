#pragma once

#include <experimental/utility>
#include <memory>

//  Problem: assigning (for example) to an element in a vector won't keep that
//  element connected in a way that it updates that vector.

namespace wayverb {
namespace combined {
namespace model {

/*
/// A shared-pointer with value semantics.
/// Useful for use with `weak_ptr`, checking for ended lifetimes etc.
template <typename T>
class shared_value final {
public:
    shared_value()
            : value_{std::make_shared<T>()} {}

    explicit shared_value(T t)
            : value_{std::make_shared<T>(std::move(t))} {}

    shared_value(const shared_value& other)
            : value_{std::make_shared<T>(*other.value_)} {}

    shared_value(shared_value&&) noexcept = default;

    //  From shared value
    shared_value& operator=(const shared_value& other) {
        *value_ = *other.value_;
        value_->notify();
        return *this;
    }

    shared_value& operator=(shared_value&& other) noexcept = default;

    //  From stack value
    shared_value& operator=(T other) {
        *value_ = std::move(other);
        value_->notify();
        return *this;
    }

    T* get() const { return value_.get(); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    const std::shared_ptr<T>& get_shared_ptr() const { return value_; }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(value_);
    }

private:
    std::shared_ptr<T> value_;
};

template <typename T>
bool operator==(const shared_value<T>& a, const shared_value<T>& b) {
    return *a == *b;
}

template <typename T>
bool operator!=(const shared_value<T>& a, const shared_value<T>& b) {
    return !(a == b);
}
*/

}  // namespace model
}  // namespace combined
}  // namespace wayverb
