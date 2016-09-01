#include "SurfaceModel.hpp"

#include "common/map_to_vector.h"
#include "common/stl_wrappers.h"

namespace model {

scene_data::material to_material(const std::pair<std::string, surface>& i) {
    return scene_data::material{i.first, i.second};
}

aligned::vector<scene_data::material> to_material_vector(
        const aligned::map<std::string, surface>& i) {
    return map_to_vector(i, to_material);
}

}  // namespace model