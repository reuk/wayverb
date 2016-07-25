#pragma once

#include "common/aligned/vector.h"

namespace raytracer {

template <typename T>
class iterative_builder final {
public:
    iterative_builder(size_t items, size_t iterations)
            : count(0)
            , depth(iterations)
            , data(items, aligned::vector<T>(iterations)) {}

    void push(const aligned::vector<T>& input) {
        if (count < depth) {
            const auto num_items = data.size();
            if (input.size() != num_items) {
                throw std::runtime_error(
                        "incorrect vector size passed to iterative_builder");
            }

            for (auto i = 0u; i != num_items; ++i) {
                data[i][count] = input[i];
            }

            count += 1;
        }
    }

    const aligned::vector<aligned::vector<T>>& get_data() const {
        if (count != depth) {
            throw std::runtime_error(
                    "iterative_builder not finished building structure");
        }

        return data;
    }

private:
    size_t count;
    size_t depth;
    aligned::vector<aligned::vector<T>> data;
};

}  // namespace raytracer
