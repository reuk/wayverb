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

template <typename Owner, typename Item>
class persistent_connection final {
public:
    using scoped_connection = typename Item::scoped_connection;

    using owner_t = Owner;
    using item_t = Item;

    /// Assumption: owner will always outlive the connection.
    explicit persistent_connection(owner_t& owner)
            : persistent_connection{owner, item_t{}} {}

    persistent_connection(owner_t& owner, const item_t& i)
            : item{i}
            , connection_{item.connect([o = &owner](auto&) { o->notify(); })} {}

    persistent_connection(const persistent_connection&) = delete;
    persistent_connection(persistent_connection&&) noexcept = delete;
    persistent_connection& operator=(const persistent_connection&) = delete;
    persistent_connection& operator=(persistent_connection&&) noexcept = delete;

    /// This is safe to have public, as long as copy-assignment notifies as
    /// expected.
    item_t item;

private:
    scoped_connection connection_;
};

struct item_extractor final {
    template <typename T, typename U>
    auto& operator()(
            const std::shared_ptr<persistent_connection<T, U>>& conn) const {
        return conn->item;
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
            : data_members_{std::make_shared<
                      persistent_connection<Derived, DataMembers>>(
                      *static_cast<Derived*>(this))...} {}

    /// Sets up each connection to forward to this object.
    /// Assumes ownership of passed-in objects.
    explicit owning_member(const DataMembers&... data_members)
            : data_members_{std::make_shared<
                      persistent_connection<Derived, DataMembers>>(
                      *static_cast<Derived*>(this), data_members)...} {}

    /// Sets up each connection to forward to this object.
    /// Deep-copies content of other object, creating new shared_ptrs to own.
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
        /// This is safe because the owning_member constructor will always
        /// allocate the member shared_ptrs - there's no danger of dereferencing
        /// a nullptr.
        util::for_each([&archive](const auto& i) { archive(i->item); },
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
        return std::get<persistent_connection<Derived, T>>(data_members_);
    }

private:
    using data_members = std::tuple<
            std::shared_ptr<persistent_connection<Derived, DataMembers>>...>;

    template <size_t I>
    using tuple_element_t = std::tuple_element_t<I, data_members>;

    template <size_t I>
    using inner_element_t = typename tuple_element_t<I>::element_type;

    template <size_t... Ix>
    owning_member(const owning_member& other, std::index_sequence<Ix...>)
            : data_members_{std::make_shared<inner_element_t<Ix>>(
                      *static_cast<Derived*>(this),
                      std::get<Ix>(other.data_members_)->item)...} {}

    template <size_t... Ix>
    void assign(const owning_member& other, std::index_sequence<Ix...>) {
        (void)std::initializer_list<int>{
                ((void)(std::get<Ix>(data_members_)->item =
                                std::get<Ix>(other.data_members_)->item),
                 0)...};
    }

    template <size_t... Ix>
    auto tie(std::index_sequence<Ix...>) const {
        return std::tie(std::get<Ix>(data_members_)->item...);
    }

    auto tie() const {
        return tie(std::make_index_sequence<sizeof...(DataMembers)>{});
    }

    data_members data_members_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
