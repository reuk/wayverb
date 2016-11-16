#pragma once

#include "utilities/event.h"
#include "utilities/for_each.h"
#include "utilities/map.h"
#include "utilities/mapping_iterator_adapter.h"

#include "cereal/access.hpp"

#include <tuple>

namespace wayverb {
namespace combined {
namespace model {

template <typename Derived>
class basic_member {
public:
    using derived_type = Derived;
    using base_type = basic_member<Derived>;

    basic_member() = default;

    /// On copy construct, do nothing.
    /// It's not possible for this object to have listeners if it's being
    /// constructed.
    basic_member(const basic_member&) {}

    /// On move construct, object should assume the lifetime of the moved-from
    /// object, and delete any previous internal state.
    /// Here, that means taking the listeners of the moved-from object.
    basic_member(basic_member&&) noexcept = default;

    /// On copy assign, assume the values of the copied-from object.
    /// Subclasses will probably want to call notify() to signal that the
    /// value has changed.
    basic_member& operator=(const basic_member&) { return *this; }

    /// On move assign, take the moved-from object's listeners.
    basic_member& operator=(basic_member&&) noexcept = default;

    using on_change = util::event<Derived&>;
    using connection = typename on_change::connection;
    using scoped_connection = typename on_change::scoped_connection;
    using callback_type = typename on_change::callback_type;

    connection connect(callback_type t) {
        return on_change_.connect(std::move(t));
    }

    void notify() { on_change_(*static_cast<Derived*>(this)); }
    size_t connections() const { return on_change_.size(); }

protected:
    ~basic_member() noexcept = default;

private:
    on_change on_change_;
};

////////////////////////////////////////////////////////////////////////////////

/// A shared-pointer with value semantics.
/// Useful for use with `weak_ptr`, checking for ended lifetimes etc.
template <typename T>
class shared_value final {
public:
    shared_value()
            : value_{std::make_shared<T>()} {}

    template <typename... Ts>
    explicit shared_value(Ts&&... ts)
            : value_{std::make_shared<T>(std::forward<Ts>(ts)...)} {}

    shared_value(const shared_value& other)
            : value_{std::make_shared<T>(*other.value_)} {}

    shared_value(shared_value&& other) noexcept
            : value_{std::make_shared<T>(std::move(*other.value_))} {}

    shared_value& operator=(const shared_value& other) {
        *value_ = *other.value_;
    }

    shared_value& operator=(shared_value&& other) noexcept {
        *value_ = std::move(*other.value_);
    }

    T* get() const { return value_.get(); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    void get_weak_ptr() const { return std::weak_ptr<T>{value_}; }

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

////////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename... DataMembers>
class owning_member;

template <typename T, size_t MinimumSize>
class vector;

template <typename T>
class item_connection final {
public:
    using scoped_connection = typename T::scoped_connection;

    item_connection() = default;

    template <typename U, typename... Ts>
    item_connection(U& owner, Ts&&... ts);

    template <typename U>
    void set_owner(U& owner);

    const shared_value<T>& get() const { return item_; }

    template <typename Archive>
    void load(Archive& archive) {
        archive(item_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(item_);
    }

private:
    shared_value<T> item_;
    scoped_connection connection_;
};

struct item_extractor final {
    template <typename U>
    constexpr const auto& operator()(U&& u) const {
        return u.get();
    }
};

template <typename It>
auto make_item_extractor_iterator(It it) {
    return util::make_mapping_iterator_adapter(std::move(it), item_extractor{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename... DataMembers>
class owning_member : public basic_member<Derived> {
public:
    using derived_type = Derived;
    using base_type = owning_member<Derived, DataMembers...>;

    owning_member()
            : data_members_{std::make_tuple(
                      item_connection<DataMembers>{*this}...)} {}

    explicit owning_member(DataMembers... data_members)
            : data_members_{std::make_tuple(item_connection<DataMembers>{
                      *this, std::move(data_members)}...)} {}

    owning_member(const owning_member& other)
            : data_members_{copy_from(other.data_members_)} {}

    owning_member(owning_member&& other) noexcept
            : data_members_{move_from(std::move(other.data_members_))} {}

    void swap(owning_member& other) noexcept {
        using std::swap;
        swap(data_members_, other.data_members_);
    }

    owning_member& operator=(const owning_member& other) {
        auto copy{other};
        swap(copy);
        set_owner();
        return *this;
    }

    owning_member& operator=(owning_member&& other) noexcept {
        swap(other);
        set_owner();
        return *this;
    }

    template <typename Archive>
    void load(Archive& archive) {
        archive(data_members_);
        set_owner();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_members_);
    }

protected:
    ~owning_member() = default;

    template <size_t I>
    const auto& get() const & {
        return std::get<I>(data_members_);
    }

    template <typename T>
    const auto& get() const & {
        return std::get<item_connection<T>>(data_members_);
    }

private:
    using data_members = std::tuple<item_connection<DataMembers>...>;

    template <size_t... Ix>
    auto copy_from(const data_members& x, std::index_sequence<Ix...>) {
        return std::make_tuple(std::tuple_element_t<Ix, data_members>{
                *this, *(std::get<Ix>(x).get())}...);
    }

    auto copy_from(const data_members& x) {
        return copy_from(
                x, std::make_index_sequence<std::tuple_size<data_members>{}>{});
    }

    template <size_t... Ix>
    auto move_from(data_members&& x, std::index_sequence<Ix...>) noexcept {
        return std::make_tuple(std::tuple_element_t<Ix, data_members>{
                *this, std::move(*(std::get<Ix>(x).get()))}...);
    }

    auto move_from(data_members&& x) noexcept {
        return move_from(
                std::move(x),
                std::make_index_sequence<std::tuple_size<data_members>{}>{});
    }

    void set_owner() {
        for_each([this](auto& i) { i.set_owner(*this); }, data_members_);
    }

    data_members data_members_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
template <typename U, typename... Ts>
item_connection<T>::item_connection(U& owner, Ts&&... ts)
        : item_{std::forward<Ts>(ts)...} {
    set_owner(owner);
}

template <typename T>
template <typename U>
void item_connection<T>::set_owner(U& owner) {
    connection_ = scoped_connection{
            item_->connect([&owner](auto&) { owner.notify(); })};
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
