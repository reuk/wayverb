#pragma once

#include "combined/model/vector.h"

namespace wayverb {
namespace combined {
namespace model {

/// A vector which must hold some minimum number of elements.
template <typename T, size_t MinSize>
class min_size_vector
        : public owning_member<min_size_vector<T, MinSize>, vector<T>> {
public:
    using base_type = owning_member<min_size_vector<T, MinSize>, vector<T>>;

    /// IMPORTANT: constructors allocate 'extra' elements *in addition to* the
    /// minimum number of elements.
    min_size_vector(size_t extra, const T& t)
            : base_type{vector<T>{MinSize + extra, t}} {}

    min_size_vector(size_t extra)
            : min_size_vector{extra, T{}} {}

    min_size_vector()
            : min_size_vector{0} {}

    template <size_t N>
    explicit min_size_vector(const T (&arr)[N])
            : base_type{vector<T>{arr}} {
        static_assert(MinSize <= N,
                      "Must be at least MinSize elements in initializer list.");
    }

    //  Extra functionality over a normal vector.

    bool can_erase() const { return MinSize < size(); }

    //  Forwarding methods to impl.

    const auto& operator[](size_t index) const { return vec()[index]; }
    auto& operator[](size_t index) { return vec()[index]; }

    auto& front() { return vec().front(); }
    const auto& front() const { return vec().front(); }

    auto& back() { return vec().back(); }
    const auto& back() const { return vec().back(); }

    auto cbegin() const { return vec().cbegin(); }
    auto begin() const { return vec().begin(); }
    auto begin() { return vec().begin(); }

    auto cend() const { return vec().cend(); }
    auto end() const { return vec().end(); }
    auto end() { return vec().end(); }

    void reserve(size_t items) { vec().reserve(items); }

    template <typename It>
    auto insert(It it, const T& t) {
        return vec().insert(std::move(it), t);
    }

    void push_back(const T& t) { vec().push_back(t); }

    void pop_back() { vec().pop_back(); }

    //  TODO could return std::optional iterator depending on whether or not
    //  erase was successful.
    template <typename It>
    void erase(It it) {
        if (can_erase()) {
            vec().erase(std::move(it));
        }
    }

    /// Will resize to std::max(MinSize, new_size)
    void resize(size_t new_size, const T& t) {
        vec().resize(std::max(MinSize, new_size), t);
    }

    void resize(size_t new_size) { resize(new_size, T{}); }

    auto size() const { return vec().size(); }
    auto empty() const { return vec().empty(); }

    void clear() { vec().clear(); }

    NOTIFYING_COPY_ASSIGN_DECLARATION(min_size_vector)
private:
    inline void swap(min_size_vector&) noexcept { using std::swap; };

    auto& vec() { return *this->template get<0>(); }
    const auto& vec() const { return *this->template get<0>(); }
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
