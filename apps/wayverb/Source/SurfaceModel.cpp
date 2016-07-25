#include "SurfaceModel.hpp"

namespace model {

SceneData::Material to_material(const std::pair<std::string, Surface>& i) {
    return SceneData::Material{i.first, i.second};
}

aligned::vector<SceneData::Material> to_material_vector(
        const aligned::map<std::string, Surface>& i) {
    aligned::vector<SceneData::Material> ret;
    ret.reserve(i.size());
    proc::transform(i, std::back_inserter(ret), to_material);
    return ret;
}

}  // namespace model