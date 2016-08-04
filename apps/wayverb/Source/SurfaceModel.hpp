#pragma once

#include "common/scene_data.h"

namespace model {

copyable_scene_data::material to_material(
        const std::pair<std::string, surface>& i);

aligned::vector<copyable_scene_data::material> to_material_vector(
        const aligned::map<std::string, surface>& i);

}  // namespace model