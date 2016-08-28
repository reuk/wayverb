#pragma once

#include "common/mapping_iterator_adapter.h"
#include "common/stl_wrappers.h"

#include <memory>

namespace raytracer {

template <template <class...> class Set,
          typename T,
          typename Comp = std::less<T>>
class recursive_set_adapter final {
public:
    recursive_set_adapter() = default;
    explicit recursive_set_adapter(Comp c)
            : data_(comp(std::move(c))) {}

    auto begin() { return make_iterator(data_.begin()); }
    const auto begin() const { return make_iterator(data_.begin()); }
    const auto cbegin() const { return make_iterator(data_.cbegin()); }

    auto end() { return make_iterator(data_.end()); }
    const auto end() const { return make_iterator(data_.end()); }
    const auto cend() const { return make_iterator(data_.cend()); }

    auto find(const T& t) const {
        const auto it{proc::lower_bound(
                data_, t, [&](const std::unique_ptr<T>& a, const T& b) {
                    return key_comp()(*a, b);
                })};
        if (it == data_.end() || values_not_equal(t, *(*it))) {
            return make_iterator(data_.end());
        }
        return make_iterator(it);
    }

    auto insert(T t) {
        const auto pair{data_.insert(std::make_unique<T>(std::move(t)))};
        return std::make_pair(make_iterator(pair.first), pair.second);
    }

    auto key_comp() const { return data_.key_comp().get_comp(); }
    auto value_comp() const { return data_.key_comp().get_comp(); }

    auto size() const { return data_.size(); }

private:
    constexpr bool values_not_equal(const T& a, const T& b) const {
        return key_comp()(a, b) || key_comp()(b, a);
    }

    class deref final {
    public:
        template <typename U>
        auto& operator()(U& t) const {
            return *t;
        }
    };

    template <typename It>
    static auto make_iterator(It it) {
        return make_mapping_iterator_adapter(std::move(it), deref{});
    }

    class comp final {
    public:
        comp() = default;
        comp(Comp comp)
                : comp_(std::move(comp)) {}

        bool operator()(const std::unique_ptr<T>& a,
                        const std::unique_ptr<T>& b) const {
            return comp_(*a, *b);
        }

        Comp get_comp() const { return comp_; }

    private:
        Comp comp_;
    };

    Set<std::unique_ptr<T>, comp> data_;
};

}  // namespace raytracer
