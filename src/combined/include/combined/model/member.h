#pragma once

#include "combined/model/shared_value.h"
#include "combined/model/shared_value_connection.h"

#include "utilities/event.h"
#include "utilities/for_each.h"
#include "utilities/map.h"

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
    basic_member(basic_member&&) noexcept = delete;

    /// On copy assign, assume the values of the copied-from object.
    /// Subclasses will probably want to call notify() to signal that the
    /// value has changed.
    basic_member& operator=(const basic_member&) { return *this; }

    /// On move assign, take the moved-from object's listeners.
    basic_member& operator=(basic_member&&) noexcept = delete;

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

template <typename Derived, typename... DataMembers>
class owning_member : public basic_member<Derived> {
public:
    using derived_type = Derived;
    using base_type = owning_member<Derived, DataMembers...>;

    owning_member()
            : data_members_{
                      std::make_tuple(item_connection<DataMembers>{}...)} {
        set_owner();
    }

    explicit owning_member(DataMembers... data_members)
            : data_members_{std::make_tuple(item_connection<DataMembers>{
                      std::move(data_members)}...)} {
        set_owner();
    }

    owning_member(const owning_member& other)
            : data_members_{other.data_members_} {
        set_owner();
    }

    /*
    owning_member(owning_member&& other) noexcept
            : data_members_{std::move(other.data_members_)} {
        set_owner();
    }
    */

private:
    void swap(owning_member& other) noexcept {
        using std::swap;
        swap(data_members_, other.data_members_);
        set_owner();
    }

public:
    owning_member& operator=(const owning_member& other) {
        auto copy{other};
        swap(copy);
        return *this;
    }

    /*
    owning_member& operator=(owning_member&& other) noexcept {
        swap(other);
        return *this;
    }
    */

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(data_members_);
        set_owner();
    }

protected:
    ~owning_member() = default;

    template <size_t I>
    auto& get() & {
        return std::get<I>(data_members_).get();
    }

    template <size_t I>
    const auto& get() const & {
        return std::get<I>(data_members_).get();
    }

    template <typename T>
    auto& get() & {
        return std::get<item_connection<T>>(data_members_).get();
    }

    template <typename T>
    const auto& get() const & {
        return std::get<item_connection<T>>(data_members_).get();
    }

private:
    using data_members = std::tuple<item_connection<DataMembers>...>;

    void set_owner() noexcept {
        util::for_each([this](auto& i) { i.set_owner(*this); }, data_members_);
    }

    data_members data_members_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
