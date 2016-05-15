#pragma once

#include "ModelWrapper.hpp"

class SurfaceModel : public model::ModelMember {
public:
    class MaterialWrapper : public model::ValueWrapper<SceneData::Material> {
    public:
        using model::ValueWrapper<SceneData::Material>::ValueWrapper;

        model::ValueWrapper<std::string> name{this, t.name};
        model::SurfaceWrapper surface{this, t.surface};
    };

    class MaterialEntry : public model::ModelMember {
    public:
        MaterialEntry(
            ModelMember* parent,
            const SceneData::Material& material = SceneData::Material());
        SceneData::Material material;
        MaterialWrapper material_wrapper{this, material};
    };

    SurfaceModel(ModelMember* parent,
                 const std::map<std::string, Surface>& init);

    SurfaceModel(ModelMember* parent,
                 const std::vector<SceneData::Material>& init);

    std::vector<std::reference_wrapper<MaterialWrapper>> get_material_wrappers()
        const;
    std::vector<SceneData::Material> get_materials() const;
    std::map<std::string, Surface> get_surfaces() const;

    auto get_num_materials() const {
        return materials.size();
    }

    MaterialWrapper& add_entry(
        const SceneData::Material& m = SceneData::Material{"new material",
                                                           Surface{}});
    void delete_entry(int index);

    const SceneData::Material& get_material_at(size_t index) const;
    const MaterialWrapper& get_material_wrapper_at(size_t index) const;

private:
    std::vector<std::unique_ptr<MaterialEntry>> materials;

    std::unique_ptr<MaterialEntry> make_material_entry(
        const SceneData::Material& init);

    std::vector<std::unique_ptr<MaterialEntry>> make_material_entries(
        const std::vector<SceneData::Material>& init);
};