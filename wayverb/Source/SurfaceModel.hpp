#pragma once

#include "common/scene_data.h"

namespace model {

SceneData::Material to_material(const std::pair<std::string, Surface>& i);
std::vector<SceneData::Material> to_material_vector(
    const std::map<std::string, Surface>& i);

}  // namespace model