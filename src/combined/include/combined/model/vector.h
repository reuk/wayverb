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
    vector() = default;

    vector(const vector&) = delete;
    vector(vector&&) noexcept = delete;

    vector& operator=(const vector&) = delete;
    vector& operator=(vector&&) noexcept = delete;

    const auto& operator[](size_t index) const { return data_[index]; }
    auto& operator[](size_t index) { return data_[index]; }

    template <typename... Ts>
    void emplace(size_t index, Ts&&... ts) {
        data_.emplace(data_.begin() + index, std::forward<Ts>(ts)...);
        data_[index].connect_on_change([&] { this->notify(); });
        this->notify();
    }

    void erase(size_t index) {
        data_.erase(data_.begin() + index);
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
