#pragma once

#include "common/aligned/vector.h"
#include "common/cl/include.h"

namespace waveguide {
namespace postprocessor {

template <typename T>
class output_accumulator final {
public:
    template <typename... Ts>
    output_accumulator(Ts&&... ts)
            : postprocessor_{std::forward<Ts>(ts)...} {}

    void operator()(cl::CommandQueue& queue,
                    const cl::Buffer& buffer,
                    size_t step) {
        output_.emplace_back(postprocessor_(queue, buffer, step));
    }

    const auto& get_output() const { return output_; }

private:
    using value_type = typename T::value_type;
    T postprocessor_;
    aligned::vector<value_type> output_;
};

}  // namespace postprocessor
}  // namespace waveguide