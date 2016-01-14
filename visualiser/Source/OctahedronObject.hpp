#pragma once

#include "BasicDrawableObject.hpp"

class OctahedronObject final : public BasicDrawableObject {
public:
    OctahedronObject(const GenericShader& shader,
                     const glm::vec3& position,
                     const glm::vec4& color);
};
