#pragma once

#include "common/scene_data.h"

namespace model {

SceneData::Material to_material(const std::pair<std::string, Surface>& i);

aligned::vector<SceneData::Material> to_material_vector(
        const aligned::map<std::string, Surface>& i);

}  // namespace model