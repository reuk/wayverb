#include "common/scene_data_loader.h"
#include "common/scene_data.h"

#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

class scene_data_loader::impl final {
public:
    impl(const std::string& scene_file) {
        const auto scene{importer.ReadFile(
                scene_file,
                (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                 aiProcess_FlipUVs))};

        if (!scene) {
            throw std::runtime_error(
                    "scene pointer is null - couldn't load scene for some "
                    "reason");
        }

        struct scene_data::contents contents {};
        contents.materials.resize(scene->mNumMaterials);
        std::transform(scene->mMaterials,
                       scene->mMaterials + scene->mNumMaterials,
                       contents.materials.begin(),
                       [](auto i) {
                           aiString material_name;
                           i->Get(AI_MATKEY_NAME, material_name);
                           return scene_data::material{material_name.C_Str(),
                                                       surface{}};
                       });

        for (auto i = 0u; i != scene->mNumMeshes; ++i) {
            auto mesh = scene->mMeshes[i];

            aligned::vector<cl_float3> vertices(mesh->mNumVertices);
            std::transform(mesh->mVertices,
                           mesh->mVertices + mesh->mNumVertices,
                           vertices.begin(),
                           [](auto i) { return to_cl_float3(i); });

            aligned::vector<triangle> triangles(mesh->mNumFaces);
            std::transform(
                    mesh->mFaces,
                    mesh->mFaces + mesh->mNumFaces,
                    triangles.begin(),
                    [&mesh, &contents](auto i) {
                        return triangle{
                                mesh->mMaterialIndex,
                                static_cast<cl_uint>(i.mIndices[0] +
                                                     contents.vertices.size()),
                                static_cast<cl_uint>(i.mIndices[1] +
                                                     contents.vertices.size()),
                                static_cast<cl_uint>(i.mIndices[2] +
                                                     contents.vertices.size())};
                    });

            contents.vertices.insert(
                    contents.vertices.end(), vertices.begin(), vertices.end());
            contents.triangles.insert(contents.triangles.end(),
                                      triangles.begin(),
                                      triangles.end());
        }

        data.set_contents(std::move(contents));
    }

    void save(const std::string& f) const {
        Assimp::Exporter().Export(importer.GetScene(), "obj", f);
    }

    const scene_data& get_scene_data() const { return data; }

private:
    Assimp::Importer importer;
    scene_data data;
};

scene_data_loader::scene_data_loader(scene_data_loader&&) noexcept = default;
scene_data_loader& scene_data_loader::operator=(scene_data_loader&&) noexcept =
        default;
scene_data_loader::~scene_data_loader() noexcept = default;

scene_data_loader::scene_data_loader(const std::string& scene_file)
        : pimpl(std::make_unique<impl>(scene_file)) {}

bool scene_data_loader::is_loaded() const { return pimpl != nullptr; }

void scene_data_loader::load(const std::string& scene_file) {
    pimpl = std::make_unique<impl>(scene_file);
}

void scene_data_loader::save(const std::string& f) const {
    if (is_loaded()) {
        pimpl->save(f);
    } else {
        throw std::logic_error("can't save if nothing's been loaded");
    }
}

void scene_data_loader::clear() { pimpl = nullptr; }

const scene_data& scene_data_loader::get_scene_data() const {
    if (is_loaded()) {
        return pimpl->get_scene_data();
    } else {
        throw std::logic_error(
                "can't access scene data if nothing's been loaded");
    }
}
