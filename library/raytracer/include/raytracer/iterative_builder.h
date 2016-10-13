#pragma once

#include "utilities/aligned/vector.h"

#include <experimental/optional>

namespace raytracer {

template <typename T>
class iterative_builder final {
public:
    iterative_builder(size_t items)
            : data(items) {}

    template <typename It, typename Func>
    void push(It b, It e, const Func& func) {
        const auto num_items{data.size()};
        if (std::distance(b, e) != num_items) {
            throw std::runtime_error{
                    "incorrect range size passed to iterative_builder"};
        }

        for (auto i{0ul}; i != num_items; ++i) {
            push_item(i, func(b[i]));
        }
    }

    template <typename It>
    void push(It b, It e) {
        push(b, e, [](const auto& i) -> const auto& { return i; });
    }

    const auto& get_data() const { return data; }
    auto& get_data() { return data; }

private:
    void push_item(size_t index, T item) {
        data[index].emplace_back(std::move(item));
    }
    void push_item(size_t index, const std::experimental::optional<T>& item) {
        if (item) {
            push_item(index, *item);
        }
    }

    aligned::vector<aligned::vector<T>> data;
};

}  // namespace raytracer
