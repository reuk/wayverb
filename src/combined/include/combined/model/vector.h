#pragma once

#include "combined/model/member.h"

#include "utilities/event.h"
#include "utilities/mapping_iterator_adapter.h"

#include <memory>

namespace wayverb {
namespace combined {
namespace model {

template <typename T>
class vector final : public basic_member<vector<T>> {
public:
    using data_member = persistent_connection<T>;

    /// So that back_inserter works.
    using value_type = T;

    vector(size_t elements, const T& t) {
        reserve(elements);
        for (auto i = 0u; i != elements; ++i) {
            data_.emplace_back(*this, t);
        }
    }

    explicit vector(size_t elements)
            : vector{elements, T{}} {}

    vector() = default;

    template <typename It>
    vector(It b, It e) {
        reserve(std::distance(b, e));
        std::copy(b, e, std::back_inserter(*this));
    }

    template <size_t N>
    explicit vector(const T (&arr)[N])
            : vector{std::begin(arr), std::end(arr)} {}

    vector(const vector& other)
            : vector{make_item_extractor_iterator(std::begin(other)),
                     make_item_extractor_iterator(std::end(other))} {}

    /// Not strongly exception safe.
    vector& operator=(const vector& other) {
        clear();
        reserve(other.size());
        std::copy(make_item_extractor_iterator(std::begin(other)),
                  make_item_extractor_iterator(std::end(other)),
                  std::back_inserter(*this));
        this->notify();
        return *this;
    }

    const auto& operator[](size_t index) const { return data_[index]; }
    auto& operator[](size_t index) { return data_[index]; }

    auto& front() { return data_.front(); }
    const auto& front() const { return data_.front(); }

    auto& back() { return data_.back(); }
    const auto& back() const { return data_.back(); }

    auto cbegin() const { return data_.cbegin(); }
    auto begin() const { return data_.begin(); }
    auto begin() { return data_.begin(); }

    auto cend() const { return data_.cend(); }
    auto end() const { return data_.end(); }
    auto end() { return data_.end(); }

    void reserve(size_t items) { data_.reserve(items); }

    template <typename It>
    auto insert(It it, const T& t) {
        const auto i = data_.emplace(std::move(it), *this, t);
        this->notify();
        return i;
    }

    void push_back(const value_type& t) {
        data_.emplace_back(*this, t);
        this->notify();
    }

    void pop_back() {
        data_.pop_back();
        this->notify();
    }

    template <typename It>
    auto erase(It it) {
        const auto i = data_.erase(std::move(it));
        this->notify();
        return i;
    }

    void resize(size_t new_size, const value_type& t) {
        reserve(new_size);
        while (new_size < size()) {
            pop_back();
        }
        while (size() < new_size) {
            push_back(t);
        }
    }

    void resize(size_t new_size) { resize(new_size, value_type{}); }

    auto size() const { return data_.size(); }
    auto empty() const { return data_.empty(); }

    void clear() {
        data_.clear();
        this->notify();
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        /// Inspired by the cereal vector serialization methods.

        cereal::size_type size = this->size();
        archive(cereal::make_size_tag(size));
        resize(size);
        for (const auto& i : *this) {
            archive(*i);
        }
    }

    bool operator==(const vector& x) const {
        //  Two vectors are equal if their values are equal, *not* their
        //  memory layout.
        return std::equal(
                cbegin(),
                cend(),
                x.cbegin(),
                x.cend(),
                [](const auto& a, const auto& b) { return *a == *b; });
    }
    bool operator!=(const vector& x) const { return !operator==(x); }

private:
    std::vector<data_member> data_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
