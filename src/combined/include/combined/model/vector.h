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
public:
    static_assert(std::is_nothrow_move_constructible<T>{} &&
                          std::is_nothrow_move_assignable<T>{},
                  "T must be nothrow moveable");

    /// item_connection is not copyable, so we can't use the normal vector
    /// constructor here.
    explicit vector(size_t extra_elements, const T& t = T()) {
        const auto target_size = MinimumSize + extra_elements;
        data_.reserve(target_size);
        for (auto i = 0; i != target_size; ++i) {
            data_.emplace_back(*this, t);
        }
    }

    explicit vector(const T& t = T())
            : vector{0, t} {}

    void swap(vector& other) noexcept {
        assert(!busy_);
        using std::swap;
        swap(data_, other.data_);
    }

    vector(const vector& other)
            : data_{other.data_} {
        set_owner();
    }

    vector(vector&& other) noexcept {
        swap(other);
        set_owner();
    }

    vector& operator=(const vector& other) {
        auto copy{other};
        swap(copy);
        set_owner();
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        swap(other);
        set_owner();
        return *this;
    }

    const auto& operator[](size_t index) const { return data_[index].get(); }

    auto cbegin() const { return make_item_extractor_iterator(data_.cbegin()); }
    auto begin() const { return make_item_extractor_iterator(data_.begin()); }
    auto begin() { return make_item_extractor_iterator(data_.begin()); }

    auto cend() const { return make_item_extractor_iterator(data_.cend()); }
    auto end() const { return make_item_extractor_iterator(data_.end()); }
    auto end() { return make_item_extractor_iterator(data_.end()); }

    template <typename It>
    void insert(It it, T t) {
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
        set_owner();
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(data_);
    }

    void set_busy(bool busy) { busy_ = busy; }
    bool get_busy() const { return busy_; }

private:
    void set_owner() {
        for (auto& i : data_) {
            i.set_owner(*this);
        }
    }

    util::aligned::vector<item_connection<T>> data_;

    /// Users can set this to ask that the vector not be updated in a way
    /// that
    /// would invalidate references into it.
    /// Obviously this is rubbish.
    bool busy_ = false;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
