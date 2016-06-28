#pragma once

#include "OtherComponents/BasicDrawableObject.hpp"

class BoxObject final : public BasicDrawableObject {
public:
    BoxObject(mglu::GenericShader& shader);
};
