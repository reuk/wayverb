#pragma once

#include "BasicDrawableObject.hpp"
#include "FadeShader.hpp"

class FrequencyAxisObject final : public BasicDrawableObject {
public:
    FrequencyAxisObject(ShaderProgram& shader);
};
