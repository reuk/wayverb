#include "SurfaceModel.hpp"

#include "common/map.h"
#include "common/stl_wrappers.h"

namespace model {

SceneData::Material to_material(const std::pair<std::string, Surface>& i) {
    return SceneData::Material{i.first, i.second};
}

aligned::vector<SceneData::Material> to_material_vector(
        const aligned::map<std::string, Surface>& i) {
    return map_to_vector(i, to_material);
}

}  // namespace model