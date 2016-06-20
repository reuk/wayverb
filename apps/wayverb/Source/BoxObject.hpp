#pragma once

#include "BasicDrawableObject.hpp"

class BoxObject final : public BasicDrawableObject {
public:
    BoxObject(mglu::GenericShader& shader);
};
