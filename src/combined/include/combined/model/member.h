#pragma once

#include "utilities/event.h"
#include "utilities/for_each.h"
#include "utilities/map.h"

#include <tuple>

namespace wayverb {
namespace combined {
namespace model {

/*
template <typename Derived>
class basic_member {
public:
    using derived_type = Derived;
    using type = basic_member<Derived>;

    basic_member() = default;
    basic_member(const basic_member&) = delete;
    basic_member(basic_member&&) noexcept = default;

    basic_member& operator=(const basic_member&) = delete;
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

template <typename Derived, typename... DataMembers>
class owning_member;

template <typename T>
class item_connection_pair final {
public:
    using scoped_connection = typename T::scoped_connection;

    template <typename U>
    item_connection_pair(basic_member<U>& owner, T t = T());

    template <typename U>
    void set_owner(basic_member<U>& owner);

    const std::shared_ptr<T>& get() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(item_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(item_);
    }

private:
    std::shared_ptr<T> item_;
    scoped_connection connection_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename... DataMembers>
class owning_member : public basic_member<Derived> {
public:
    using derived_type = Derived;
    using type = owning_member<Derived>;

    owning_member()
            : data_members_{std::make_tuple(
                      item_connection_pair<DataMembers>{}...)} {}

    explicit owning_member(DataMembers... data_members)
            : data_members_{std::make_tuple(item_connection_pair<DataMembers>{
                      *this, std::move(data_members)}...)} {}

    void swap(owning_member& other) noexcept {
        using std::swap;
        swap(data_members_, other.data_members_);
    }

    owning_member(owning_member&& other) noexcept
            : data_members_{std::move(other.data_members_)} {
        set_owner();
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
        return std::get<I>(data_members_).get();
    }

    template <typename T>
    const auto& get() const & {
        return std::get<item_connection_pair<T>>(data_members_).get();
    }

private:
    void set_owner() {
        for_each([this](auto& i) { i.set_owner(*this); }, data_members_);
    }

    std::tuple<item_connection_pair<DataMembers>...> data_members_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
template <typename U>
item_connection_pair<T>::item_connection_pair(basic_member<U>& owner, T t)
        : item_{std::make_shared<T>(std::move(t))} {
    set_owner(owner);
}

template <typename T>
template <typename U>
void item_connection_pair<T>::set_owner(basic_member<U>& owner) {
    connection_ = scoped_connection{
            item_->connect([&owner](auto&) { owner.notify(); })};
}

template <typename T>
const std::shared_ptr<T>& item_connection_pair<T>::get() const {
    return item_;
}
*/

template <typename Derived>
class basic_member {
public:
    using derived_type = Derived;
    using type = basic_member<Derived>;

    basic_member() = default;

    basic_member(const basic_member&) {}
    basic_member(basic_member&&) noexcept = default;

    basic_member& operator=(const basic_member&) {
        notify();
        return *this;
    }

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

template <typename Derived, typename... DataMembers>
class owning_member : public basic_member<Derived> {
public:
    using derived_type = Derived;
    using type = owning_member<Derived, DataMembers...>;

    owning_member() = default;

    explicit owning_member(DataMembers... data_members)
            : data_members_{std::make_tuple(std::move(data_members)...)} {}

    owning_member(const owning_member& other)
            : data_members_{other.data_members_} {}

    owning_member(owning_member&& other) noexcept
            : data_members_{std::move(other.data_members_)} {}

    void swap(owning_member& other) noexcept {
        using std::swap;
        swap(data_members_, other.data_members_);
    }

    owning_member& operator=(const owning_member& other) {
        auto copy{other};
        swap(copy);
        scoped_connections_ = make_connections();
        this->notify();
        return *this;
    }

    owning_member& operator=(owning_member&& other) noexcept {
        swap(other);
        scoped_connections_ = make_connections();
        return *this;
    }

    template <typename Archive>
    void load(Archive& archive) {
        archive(data_members_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_members_);
    }

protected:
    ~owning_member() noexcept = default;

    template <size_t I>
    auto& get() & {
        return std::get<I>(data_members_);
    }

    template <size_t I>
    const auto& get() const & {
        return std::get<I>(data_members_);
    }

    template <typename T>
    auto& get() & {
        return std::get<T>(data_members_);
    }

    template <typename T>
    const auto& get() const & {
        return std::get<T>(data_members_);
    }

private:
    template <size_t... Ix>
    auto make_connections(std::index_sequence<Ix...>) {
        return std::make_tuple([&](auto& param) {
            return typename
std::decay_t<decltype(param)>::scoped_connection{
                    param.connect([&](auto&) { this->notify(); })};
        }(std::get<Ix>(data_members_))...);
    }

    auto make_connections() {
        return make_connections(
                std::make_index_sequence<sizeof...(DataMembers)>{});
    }

    std::tuple<DataMembers...> data_members_;
    std::tuple<typename DataMembers::scoped_connection...>
scoped_connections_ =
            make_connections();
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
