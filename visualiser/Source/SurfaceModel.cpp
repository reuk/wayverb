#include "SurfaceModel.hpp"

static SceneData::Material to_material(
    const std::pair<std::string, Surface>& i) {
    return SceneData::Material{i.first, i.second};
}

static std::vector<SceneData::Material> to_material_vector(
    const std::map<std::string, Surface>& i) {
    std::vector<SceneData::Material> ret;
    std::transform(i.begin(), i.end(), std::back_inserter(ret), to_material);
    return ret;
}

model::Surfaces::Surfaces(ModelMember* parent,
                          const std::map<std::string, Surface>& init)
        : Surfaces(parent, to_material_vector(init)) {
}

model::Surfaces::Surfaces(ModelMember* parent,
                          const std::vector<SceneData::Material>& init)
        : ValueWithWrapper(parent, init) {
}