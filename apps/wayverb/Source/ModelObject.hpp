#pragma once

#include "OtherComponents/BasicDrawableObject.hpp"

#include "common/scene_data.h"

class ModelObject final : public BasicDrawableObject {
public:
    ModelObject(mglu::GenericShader& shader, const SceneData& scene_data);
};
