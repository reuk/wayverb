#pragma once

#include "BasicDrawableObject.hpp"

class AxesObject final : public BasicDrawableObject {
public:
    AxesObject(MatrixTreeNode* parent, GenericShader& shader);
};