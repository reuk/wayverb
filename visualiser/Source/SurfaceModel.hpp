#pragma once

#include "Collection.h"
#include "ModelWrapper.hpp"

namespace model {

class Surfaces : public ValueWithWrapper<std::vector<SceneData::Material>> {
public:
    Surfaces(ModelMember* parent, const std::map<std::string, Surface>& init);
    Surfaces(ModelMember* parent, const std::vector<SceneData::Material>& init);
};

}  // namespace model