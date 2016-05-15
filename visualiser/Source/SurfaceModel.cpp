#include "SurfaceModel.hpp"

SurfaceModel::MaterialEntry::MaterialEntry(ModelMember* parent,
                                           const SceneData::Material& material)
        : ModelMember(parent)
        , material(material) {
}

//----------------------------------------------------------------------------//

std::unique_ptr<SurfaceModel::MaterialEntry> SurfaceModel::make_material_entry(
    const SceneData::Material& init) {
    return std::make_unique<MaterialEntry>(this, init);
}

std::vector<std::unique_ptr<SurfaceModel::MaterialEntry>>
SurfaceModel::make_material_entries(
    const std::vector<SceneData::Material>& init) {
    std::vector<std::unique_ptr<SurfaceModel::MaterialEntry>> ret;
    std::transform(init.begin(),
                   init.end(),
                   std::back_inserter(ret),
                   [this](const auto& i) { return make_material_entry(i); });
    return ret;
}

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

SurfaceModel::SurfaceModel(ModelMember* parent,
                           const std::map<std::string, Surface>& init)
        : SurfaceModel(parent, to_material_vector(init)) {
}

SurfaceModel::SurfaceModel(ModelMember* parent,
                           const std::vector<SceneData::Material>& init)
        : ModelMember(parent)
        , materials(make_material_entries(init)) {
}

std::vector<std::reference_wrapper<SurfaceModel::MaterialWrapper>>
SurfaceModel::get_material_wrappers() const {
    std::vector<std::reference_wrapper<MaterialWrapper>> ret;
    std::transform(materials.begin(),
                   materials.end(),
                   std::back_inserter(ret),
                   [](const auto& i) { return std::ref(i->material_wrapper); });
    return ret;
}

std::vector<SceneData::Material> SurfaceModel::get_materials() const {
    std::vector<SceneData::Material> ret;
    std::transform(materials.begin(),
                   materials.end(),
                   std::back_inserter(ret),
                   [](const auto& i) { return i->material; });
    return ret;
}

std::map<std::string, Surface> SurfaceModel::get_surfaces() const {
    std::map<std::string, Surface> ret;
    std::for_each(materials.begin(), materials.end(), [&ret](const auto& i) {
        ret[i->material.name] = i->material.surface;
    });
    return ret;
}

SurfaceModel::MaterialWrapper& SurfaceModel::add_entry(
    const SceneData::Material& m) {
    auto ptr = make_material_entry(m);
    auto& ret = ptr->material_wrapper;
    materials.push_back(std::move(ptr));
    notify();
    return ret;
}
void SurfaceModel::delete_entry(int index) {
    materials.erase(materials.begin() + index);
    notify();
}

const SceneData::Material& SurfaceModel::get_material_at(size_t index) const {
    return get_material_wrapper_at(index).get_value();
}

const SurfaceModel::MaterialWrapper& SurfaceModel::get_material_wrapper_at(
    size_t index) const {
    return materials[index]->material_wrapper;
}