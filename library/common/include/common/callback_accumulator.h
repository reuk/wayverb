#pragma once

#include "utilities/aligned/vector.h"

template <typename T, typename Ret = typename T::return_type>
class callback_accumulator final {
public:
    callback_accumulator(T t)
            : postprocessor_{std::move(t)} {}

    template <typename... Ts>
    callback_accumulator(Ts&&... ts)
            : postprocessor_{std::forward<Ts>(ts)...} {}

    template <typename... Ts>
    void operator()(Ts&&... ts) {
        output_.emplace_back(postprocessor_(std::forward<Ts>(ts)...));
    }

    const auto& get_output() const { return output_; }

private:
    aligned::vector<Ret> output_;
    T postprocessor_;
};

template <typename T>
auto make_callback_accumulator(T t) {
    return callback_accumulator<T>{std::move(t)};
}
