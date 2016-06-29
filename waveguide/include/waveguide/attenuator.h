#pragma once

#include "waveguide.h"

#include <vector>

class Attenuator {
public:
    Attenuator() = default;
    Attenuator(const Attenuator&) = default;
    Attenuator& operator=(const Attenuator&) = default;
    Attenuator(Attenuator&&) noexcept = default;
    Attenuator& operator=(Attenuator&&) noexcept= default;
    virtual ~Attenuator() noexcept = default;

    virtual std::vector<float> process(
            const std::vector<RunStepResult>& input) const = 0;
};
