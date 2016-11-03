#include "SurfaceModel.hpp"

#include "utilities/map_to_vector.h"

namespace model {

wayverb::core::scene_data_loader::material to_material(const std::pair<std::string, wayverb::core::surface<wayverb::core::simulation_bands>>& i) {
    return {i.first, i.second};
}

util::aligned::vector<wayverb::core::scene_data_loader::material> to_material_vector(
        const util::aligned::map<std::string, wayverb::core::surface<wayverb::core::simulation_bands>>& i) {
    return util::map_to_vector(begin(i), end(i), to_material);
}

}  // namespace model
