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

    explicit vector(size_t extra_elements, const T& t)
            : data_{MinimumSize + extra_elements, item_connection<T>{t}} {
        set_owner();
    }

    explicit vector(size_t extra_elements)
            : data_{MinimumSize + extra_elements} {
        set_owner();
    }

    explicit vector(const T& t)
            : vector{0, t} {}

    vector()
            : vector{0} {}

private:
    void swap(vector& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        set_owner();
    }

public:
    vector(const vector& other)
            : data_{other.data_} {
        set_owner();
    }

    vector(vector&& other) noexcept
            : data_{std::move(other.data_)} {
        set_owner();
    }

    vector& operator=(const vector& other) {
        auto copy{other};
        swap(copy);
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        swap(other);
        return *this;
    }

    const auto& operator[](size_t index) const { return data_[index].get(); }
    auto& operator[](size_t index) { return data_[index].get(); }

    auto cbegin() const { return make_item_extractor_iterator(data_.cbegin()); }
    auto begin() const { return make_item_extractor_iterator(data_.begin()); }
    auto begin() { return make_item_extractor_iterator(data_.begin()); }

    auto cend() const { return make_item_extractor_iterator(data_.cend()); }
    auto end() const { return make_item_extractor_iterator(data_.end()); }
    auto end() { return make_item_extractor_iterator(data_.end()); }

    template <typename It>
    void insert(It it, T t) {
        const auto i = data_.emplace(it.base(), std::move(t));
        i->connect(*this);
        this->notify();
    }

    template <typename It>
    void erase(It it) {
        if (can_erase()) {
            data_.erase(it.base());
            this->notify();
        }
    }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        data_.clear();
        this->notify();
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

private:
    void set_owner() {
        for (auto& i : data_) {
            i.set_owner(*this);
        }
    }

    util::aligned::vector<item_connection<T>> data_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
