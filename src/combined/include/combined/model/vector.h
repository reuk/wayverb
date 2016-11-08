#pragma once

#include "combined/model/member.h"

#include "utilities/event.h"
#include "utilities/map_to_vector.h"

#include <memory>

namespace wayverb {
namespace combined {
namespace model {

template <typename T>
class vector final : public member<vector<T>> {
public:
    static_assert(std::is_nothrow_move_constructible<T>{} &&
                          std::is_nothrow_move_assignable<T>{},
                  "T must be nothrow moveable");

    vector() = default;

    const auto& operator[](size_t index) const { return data_[index]; }
    auto& operator[](size_t index) { return data_[index]; }

    auto cbegin() const { return data_.cbegin(); }
    auto begin() const { return data_.begin(); }
    auto begin() { return data_.begin(); }

    auto cend() const { return data_.cend(); }
    auto end() const { return data_.end(); }
    auto end() { return data_.end(); }

    template <typename It>
    void insert(It it, T t) {
        t.connect_on_change([&](auto&) { this->notify(); });
        data_.insert(std::move(it), std::move(t));
        this->notify();
    }

    template <typename It>
    void erase(It it) {
        data_.erase(std::move(it));
        this->notify();
    }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        data_.clear();
        this->notify();
    }

private:
    std::vector<T> data_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
