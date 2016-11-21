#pragma once

#include "combined/model/shared_value.h"

#include "utilities/mapping_iterator_adapter.h"

namespace wayverb {
namespace combined {
namespace model {

/*
template <typename T>
class item_connection final {
public:
    using scoped_connection = typename T::scoped_connection;

    item_connection() = default;

    explicit item_connection(T t)
            : item_{std::move(t)} {}

    item_connection(const item_connection& other)
            : item_{other.item_} {}

    item_connection(item_connection&&) noexcept = default;

    void swap(item_connection& other) noexcept {
        using std::swap;
        swap(item_, other.item_);
        swap(connection_, other.connection_);
    }

    item_connection& operator=(const item_connection& other) {
        auto copy{other};
        swap(copy);
        return *this;
    }

    item_connection& operator=(item_connection&&) noexcept = default;

    template <typename U>
    void set_owner(U& owner) noexcept {
        connection_ = scoped_connection{
                item_->connect([&owner](auto&) { owner.notify(); })};
    }

    const shared_value<T>& get() const { return item_; }
    shared_value<T>& get() { return item_; }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(item_);
    }

private:
    shared_value<T> item_;
    scoped_connection connection_;
};

template <typename T>
bool operator==(const item_connection<T>& a, const item_connection<T>& b) {
    return a.get() == b.get();
}

template <typename T>
bool operator!=(const item_connection<T>& a, const item_connection<T>& b) {
    return !(a == b);
}

////////////////////////////////////////////////////////////////////////////////

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

struct connection_creator final {
    template <typename T>
    constexpr auto operator()(T t) const {
        return item_connection<T>{std::move(t)};
    }
};

template <typename It>
auto make_connection_creator_iterator(It it) {
    return util::make_mapping_iterator_adapter(std::move(it),
                                               connection_creator{});
}
*/

template <typename Owner, typename Item>
class persistent_connection final {
public:
    using scoped_connection = typename Item::scoped_connection;

    using owner_t = Owner;
    using item_t = Item;

    /// Assumption: owner will always outlive the connection.
    //explicit persistent_connection(owner_t& owner)
    //        : owner_{owner} {};

    persistent_connection(owner_t& owner, std::shared_ptr<item_t> item)
            : owner_{owner}
            , item_{std::move(item)} {
        connect();
    }

    std::shared_ptr<item_t> get() const { return item_; }
    void set(std::shared_ptr<item_t> ptr) {
        //  TODO should I explicitly disconnect first?
        item_ = std::move(ptr);
        connect();
    }

private:
    void connect() {
        connection_ = scoped_connection{
                item_->connect([this](auto&) { owner_.notify(); })};
    }

    owner_t& owner_;
    std::shared_ptr<item_t> item_;
    scoped_connection connection_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
