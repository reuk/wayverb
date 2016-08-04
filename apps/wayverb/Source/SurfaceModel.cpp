#include "SurfaceModel.hpp"

#include "common/map.h"
#include "common/stl_wrappers.h"

namespace model {

copyable_scene_data::material to_material(
        const std::pair<std::string, surface>& i) {
    return copyable_scene_data::material{i.first, i.second};
}

aligned::vector<copyable_scene_data::material> to_material_vector(
        const aligned::map<std::string, surface>& i) {
    return map_to_vector(i, to_material);
}

}  // namespace model