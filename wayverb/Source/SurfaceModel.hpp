#pragma once

#include "common/scene_data.h"

namespace model {

scene_data::material to_material(const std::pair<std::string, surface>& i);

aligned::vector<scene_data::material> to_material_vector(
        const aligned::map<std::string, surface>& i);

}  // namespace model