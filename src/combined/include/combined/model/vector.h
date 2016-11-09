#pragma once

#include "combined/model/member.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"
#include "utilities/mapping_iterator_adapter.h"

#include <memory>

namespace wayverb {
namespace combined {
namespace model {

template <typename T>
class vector final : public member<vector<T>> {
    class item_connection final {
    public:
        T item;
        typename util::event<T&>::scoped_connection connection;

        item_connection() = default;
        item_connection(vector& v, T t)
                : item{std::move(t)}
                , connection{
                          item.connect_on_change([&](auto&) { v.notify(); })} {}

        item_connection(const item_connection& other)
                : item{other.item} {}
        item_connection(item_connection&&) noexcept = default;

        item_connection& operator=(const item_connection& other) = default;
        item_connection& operator=(item_connection&&) noexcept = default;

        template <typename Archive>
        void load(Archive& archive) {
            archive(item);
        }

        template <typename Archive>
        void save(Archive& archive) const {
            archive(item);
        }
    };

    struct item_extractor final {
        template <typename U>
        constexpr auto& operator()(U&& u) const {
            return u.item;
        }
    };

    template <typename It>
    static auto make_extractor_iterator(It it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   item_extractor{});
    }

    struct item_creator final {
        vector& v;

        template <typename U>
        constexpr auto operator()(U&& u) const {
            return item_connection{v, std::forward<U>(u)};
        }
    };

    template <typename It>
    auto make_creator_iterator(It it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   item_creator{*this});
    }

public:
    static_assert(std::is_nothrow_move_constructible<T>{} &&
                          std::is_nothrow_move_assignable<T>{},
                  "T must be nothrow moveable");

    vector() = default;

    template <typename It>
    vector(It b, It e) {
        std::copy(make_creator_iterator(std::move(b)),
                  make_creator_iterator(std::move(e)),
                  std::back_inserter(data_));
    }

    void swap(vector& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
    }

    vector(const vector& other)
            : data_{other.data_} {
        connect();
    }

    vector(vector&& other) noexcept {
        swap(other);
        connect();
    }

    vector& operator=(const vector& other) {
        auto copy{other};
        swap(copy);
        connect();
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        swap(other);
        connect();
        return *this;
    }

    const auto& operator[](size_t index) const { return data_[index].item; }
    auto& operator[](size_t index) { return data_[index].item; }

    auto cbegin() const { return make_extractor_iterator(data_.cbegin()); }
    auto begin() const { return make_extractor_iterator(data_.begin()); }
    auto begin() { return make_extractor_iterator(data_.begin()); }

    auto cend() const { return make_extractor_iterator(data_.cend()); }
    auto end() const { return make_extractor_iterator(data_.end()); }
    auto end() { return make_extractor_iterator(data_.end()); }

    template <typename It>
    void insert(It it, T t) {
        data_.emplace(it.base(), *this, std::move(t));
        this->notify();
    }

    template <typename It>
    void erase(It it) {
        data_.erase(it.base());
        this->notify();
    }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        data_.clear();
        this->notify();
    }

    template <typename Archive>
    void load(Archive& archive) {
        archive(data_);
        connect();
        this->notify();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_);
    }

private:
    void connect() {
        for (auto& i : data_) {
            i.connection = typename util::event<T&>::scoped_connection{
                    i.item.connect_on_change([&](auto&) { this->notify(); })};
        }
    }

    util::aligned::vector<item_connection> data_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
