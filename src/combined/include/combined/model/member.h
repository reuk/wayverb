#pragma once

#include "utilities/event.h"
#include "utilities/for_each.h"
#include "utilities/map.h"
#include "utilities/mapping_iterator_adapter.h"

#include "cereal/types/tuple.hpp"

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
    basic_member& operator=(const basic_member&) { return *this; }

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

#define NOTIFYING_COPY_ASSIGN_DECLARATION(TYPE) \
    inline TYPE& operator=(const TYPE& other) { \
        auto copy{other};                       \
        base_type::operator=(copy);             \
        this->swap(copy);                       \
        this->notify();                         \
        return *this;                           \
    }

////////////////////////////////////////////////////////////////////////////////

template <typename Item>
class persistent_connection final {
public:
    using scoped_connection = typename Item::scoped_connection;

    using item_t = Item;

    /// Construction: item is constructed, and connected to supplied owner.
    /// Copy construction: disabled.
    /// Move construction: item & connection are moved, owner ref is unchanged.
    /// Copy assignment: disabled.
    /// Move assignment: item & connection are moved, owner ref is unchanged.
    /// Destruction: item ref count is decreased, connection to owner is
    /// severed.

    /// Assumption: owner will always outlive the connection.
    template <typename Owner>
    explicit persistent_connection(Owner& owner)
            : persistent_connection{owner, item_t{}} {}

    template <typename Owner>
    persistent_connection(Owner& owner, const item_t& i)
            : item_{std::make_shared<item_t>(i)}
            , connection_{
                      item_->connect([o = &owner](auto&) { o->notify(); })} {}

    persistent_connection(const persistent_connection&) = delete;
    persistent_connection(persistent_connection&&) noexcept = default;

    persistent_connection& operator=(const persistent_connection&) = delete;
    persistent_connection& operator=(persistent_connection&&) noexcept =
            default;

    /// Can't modify the shared ptr itself, but modifications to pointed-to
    /// object is fine.
    /// If the pointed-to object is assigned-to, it will retain its listeners
    /// from before the assignment, so this object's invariants will not be
    /// violated.
    const std::shared_ptr<item_t>& item() const { return item_; }

    auto get() const { return item_.get(); }
    auto operator-> () const { return get(); }
    auto& operator*() const { return *get(); }

private:
    std::shared_ptr<item_t> item_;
    scoped_connection connection_;
};

struct item_extractor final {
    template <typename T>
    auto& operator()(const persistent_connection<T>& conn) const {
        return *conn;
    }
};

template <typename It>
static auto make_item_extractor_iterator(It it) {
    return util::make_mapping_iterator_adapter(std::move(it), item_extractor{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename... DataMembers>
class owning_member : public basic_member<Derived> {
public:
    using derived_type = Derived;
    using base_type = owning_member<Derived, DataMembers...>;

    /// Sets up each connection to forward to this object.
    /// Default-constructs members.
    owning_member()
            : data_members_{persistent_connection<DataMembers>{
                      *static_cast<Derived*>(this)}...} {}

    /// Sets up each connection to forward to this object.
    /// Assumes ownership of passed-in objects.
    explicit owning_member(const DataMembers&... data_members)
            : data_members_{persistent_connection<DataMembers>{
                      *static_cast<Derived*>(this), data_members}...} {}

    /// Sets up each connection to forward to this object.
    /// Deep-copies content of other object.
    owning_member(const owning_member& other)
            : owning_member{
                      other,
                      std::make_index_sequence<sizeof...(DataMembers)>{}} {}

    /// Not strongly exception-safe.
    /// Elementwise deep copy from other to this.
    /// Subclasses should call notify afterwards.
    owning_member& operator=(const owning_member& other) {
        assign(other, std::make_index_sequence<sizeof...(DataMembers)>{});
        return *this;
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        /// We don't really want to serialize each persistent_connection.
        /// Instead, we directly serialize the pointed-to items.
        util::for_each(
                [&archive](const auto& i) { archive(item_extractor{}(i)); },
                data_members_);
    }

    //  Implemented as member functions because they need to poke at private
    //  data members.
    bool operator==(const owning_member& other) const {
        return tie() == other.tie();
    }

    bool operator!=(const owning_member& other) const {
        return !operator==(other);
    }

protected:
    ~owning_member() = default;

    template <size_t I>
    const auto& get() const & {
        return std::get<I>(data_members_);
    }

    template <typename T>
    const auto& get() const & {
        return std::get<persistent_connection<T>>(data_members_);
    }

private:
    using data_members = std::tuple<persistent_connection<DataMembers>...>;

    template <size_t I>
    using tuple_element_t = std::tuple_element_t<I, data_members>;

    template <size_t... Ix>
    owning_member(const owning_member& other, std::index_sequence<Ix...>)
            : data_members_{tuple_element_t<Ix>{
                      *static_cast<Derived*>(this),
                      item_extractor{}(std::get<Ix>(other.data_members_))}...} {
    }

    template <size_t... Ix>
    void assign(const owning_member& other, std::index_sequence<Ix...>) {
        (void)std::initializer_list<int>{
                ((void)(item_extractor{}(std::get<Ix>(data_members_)) =
                                item_extractor{}(
                                        std::get<Ix>(other.data_members_))),
                 0)...};
    }

    template <size_t... Ix>
    auto tie(std::index_sequence<Ix...>) const {
        return std::tie(item_extractor{}(std::get<Ix>(data_members_))...);
    }

    auto tie() const {
        return tie(std::make_index_sequence<sizeof...(DataMembers)>{});
    }

    data_members data_members_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
