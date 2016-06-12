#pragma once

#include "BasicDrawableObject.hpp"

class BoxObject final : public BasicDrawableObject {
public:
    BoxObject(MatrixTreeNode* parent, GenericShader& shader);
};
