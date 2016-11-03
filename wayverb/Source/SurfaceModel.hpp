#pragma once

#include "core/scene_data_loader.h"
#include "utilities/aligned/map.h"

namespace model {

wayverb::core::scene_data_loader::material to_material(const std::pair<std::string, wayverb::core::surface<wayverb::core::simulation_bands>>& i);

util::aligned::vector<wayverb::core::scene_data_loader::material> to_material_vector(
        const util::aligned::map<std::string, wayverb::core::surface<wayverb::core::simulation_bands>>& i);

}  // namespace model
