#pragma once

#include "common/aligned/vector.h"

#include <experimental/optional>

namespace raytracer {

template <typename T>
class iterative_builder final {
public:
    iterative_builder(size_t items)
            : count(0)
            , data(items) {
    }

    template <typename U, typename Func>
    void push(const aligned::vector<U>& input, Func func) {
        const auto num_items = data.size();
        if (input.size() != num_items) {
            throw std::runtime_error(
                    "incorrect vector size passed to iterative_builder");
        }

        for (auto i = 0u; i != num_items; ++i) {
            push_item(i, func(input[i]));
        }

        count += 1;
    }

    template <typename U>
    void push(const aligned::vector<U>& input) {
        push(input, [](const auto& i) { return i; });
    }

    const aligned::vector<aligned::vector<T>>& get_data() const { return data; }
    aligned::vector<aligned::vector<T>>& get_data() { return data; }

private:
    void push_item(size_t index, const T& item) { data[index].push_back(item); }
    void push_item(size_t index, const std::experimental::optional<T>& item) {
        if (item) {
            push_item(index, *item);
        }
    }

    size_t count;
    aligned::vector<aligned::vector<T>> data;
};

}  // namespace raytracer