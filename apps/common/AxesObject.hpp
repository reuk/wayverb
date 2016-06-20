#pragma once

#include "BasicDrawableObject.hpp"

class AxesObject final : public BasicDrawableObject {
public:
    AxesObject(mglu::GenericShader& shader);
};