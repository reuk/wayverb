#pragma once

#include "waveguide.h"

#include "common/vec.h"

#include <vector>

class Attenuator {
public:
    virtual ~Attenuator() noexcept = default;
    virtual std::vector<float> process(
        const std::vector<RunStepResult>& input) const = 0;
};
