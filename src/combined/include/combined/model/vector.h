#pragma once

#include "combined/model/member.h"

#include "utilities/aligned/vector.h"
#include "utilities/event.h"
#include "utilities/mapping_iterator_adapter.h"

#include <memory>

namespace wayverb {
namespace combined {
namespace model {

template <typename T, size_t MinimumSize>
class vector final : public basic_member<vector<T, MinimumSize>> {
    class item_connection final {
    public:
        explicit item_connection(T t = T())
                : item{std::move(t)} {}

        void swap(item_connection& other) noexcept {
            using std::swap;
            swap(item, other.item);
            swap(connection_, other.connection_);
        }

        item_connection(const item_connection& other) : item{other.item} {}
        item_connection(item_connection&& other) noexcept
                : item{std::move(other.item)}
                , connection_{std::move(other.connection_)} {}

        item_connection& operator=(const item_connection& other) {
            auto copy{other};
            swap(copy);
            return *this;
        }
        item_connection& operator=(item_connection&& other) noexcept {
            swap(other);
            return *this;
        }

        void connect(vector& v) {
            connection_ = typename T::scoped_connection{
                    item.connect([&](auto&) { v.notify(); })};
        }

        template <typename Archive>
        void load(Archive& archive) {
            archive(item);
        }

        template <typename Archive>
        void save(Archive& archive) const {
            archive(item);
        }

        T item;

    private:
        typename T::scoped_connection connection_;
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
            item_connection ret{std::forward<U>(u)};
            ret.connect(v);
            return ret;
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

    template <typename... Ts>
    vector(Ts&&... ts)
            : data_{MinimumSize, item_connection(T(std::forward<Ts>(ts)...))} {
        connect_all();
    }

    template <typename It>
    vector(It b, It e) {
        if (std::distance(b, e) < MinimumSize) {
            throw std::logic_error{
                    "must init with at least 'MinimumSize' elements"};
        }
        std::copy(make_creator_iterator(std::move(b)),
                  make_creator_iterator(std::move(e)),
                  std::back_inserter(data_));
    }

    void swap(vector& other) noexcept {
        assert(! busy_);
        using std::swap;
        swap(data_, other.data_);
    }

    vector(const vector& other)
            : data_{other.data_} {
        connect_all();
    }

    vector(vector&& other) noexcept {
        swap(other);
        connect_all();
    }

    vector& operator=(const vector& other) {
        auto copy{other};
        swap(copy);
        connect_all();
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        swap(other);
        connect_all();
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
    void insert(It it, T t = T()) {
        if (!busy_) {
            const auto i = data_.emplace(it.base(), std::move(t));
            i->connect(*this);
            this->notify();
        }
    }

    template <typename It>
    void erase(It it) {
        if (!busy_ && can_erase()) {
            data_.erase(it.base());
            this->notify();
        }
    }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        if (!busy_) {
            data_.clear();
            this->notify();
        }
    }

    bool can_erase() const { return MinimumSize < size(); }

    template <typename Archive>
    void load(Archive& archive) {
        archive(data_);
        connect_all();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_);
    }

    void set_busy(bool busy) { busy_ = busy; }
    bool get_busy() const { return busy_; }

private:
    void connect_all() {
        for (auto& i : data_) {
            i.connect(*this);
        }
    }

    util::aligned::vector<item_connection> data_;
    
    /// Users can set this to ask that the vector not be updated in a way that
    /// would invalidate references into it.
    /// Obviously this is rubbish.
    bool busy_ = false;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
