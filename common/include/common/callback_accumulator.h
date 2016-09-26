#pragma once

#include "common/aligned/vector.h"

template <typename Ret, typename T>
class callback_accumulator final {
public:
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

