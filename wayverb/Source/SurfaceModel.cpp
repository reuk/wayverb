#include "SurfaceModel.hpp"

namespace model {

SceneData::Material to_material(const std::pair<std::string, Surface>& i) {
    return SceneData::Material{i.first, i.second};
}

std::vector<SceneData::Material> to_material_vector(
    const std::map<std::string, Surface>& i) {
    std::vector<SceneData::Material> ret;
    std::transform(i.begin(), i.end(), std::back_inserter(ret), to_material);
    return ret;
}

}  // namespace model