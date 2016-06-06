#pragma once

#include "BasicDrawableObject.hpp"

#include "common/scene_data.h"

class ModelObject final : public BasicDrawableObject {
public:
    ModelObject(const GenericShader& shader, const SceneData& scene_data);

private:
    std::vector<GLuint> get_indices(const SceneData& scene_data) const;
};
