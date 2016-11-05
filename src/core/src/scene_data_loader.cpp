#include "core/scene_data_loader.h"
#include "core/conversions.h"
#include "core/scene_data.h"

#include "utilities/map_to_vector.h"

#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace wayverb {
namespace core {

class scene_data_loader::impl final {
public:
    auto load_from_file(const std::string& scene_file) {
        const auto scene = importer_.ReadFile(
                scene_file,
                (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                 aiProcess_FlipUVs));

        if (scene == nullptr) {
            throw std::runtime_error{
                    "scene pointer is null - couldn't load scene for some "
                    "reason"};
        }

        util::aligned::vector<triangle> triangles{};
        util::aligned::vector<cl_float3> vertices{};

        auto materials = util::map_to_vector(
                scene->mMaterials,
                scene->mMaterials + scene->mNumMaterials,
                [](auto i) {
                    aiString material_name;
                    i->Get(AI_MATKEY_NAME, material_name);
                    return material{material_name.C_Str(), surface<8>{}};
                });

        for (auto i = 0u; i != scene->mNumMeshes; ++i) {
            const auto mesh = scene->mMeshes[i];

            auto mesh_vertices =
                    util::map_to_vector(mesh->mVertices,
                                        mesh->mVertices + mesh->mNumVertices,
                                        [](auto i) { return to_cl_float3(i); });
            auto mesh_triangles = util::map_to_vector(
                    mesh->mFaces, mesh->mFaces + mesh->mNumFaces, [&](auto i) {
                        return triangle{mesh->mMaterialIndex,
                                        static_cast<cl_uint>(i.mIndices[0] +
                                                             vertices.size()),
                                        static_cast<cl_uint>(i.mIndices[1] +
                                                             vertices.size()),
                                        static_cast<cl_uint>(i.mIndices[2] +
                                                             vertices.size())};
                    });

            vertices.insert(
                    vertices.end(), mesh_vertices.begin(), mesh_vertices.end());
            triangles.insert(triangles.end(),
                             mesh_triangles.begin(),
                             mesh_triangles.end());
        }

        return make_scene_data(std::move(triangles),
                               std::move(vertices),
                               std::move(materials));
    }

    impl(const std::string& scene_file)
            : data_{load_from_file(scene_file)} {}

    void save(const std::string& f) const {
        Assimp::Exporter().Export(importer_.GetScene(), "obj", f);
    }

    const scene_data& get_scene_data() const { return data_; }

private:
    Assimp::Importer importer_;
    scene_data data_;
};

scene_data_loader::scene_data_loader(scene_data_loader&&) noexcept = default;
scene_data_loader& scene_data_loader::operator=(scene_data_loader&&) noexcept =
        default;
scene_data_loader::~scene_data_loader() noexcept = default;

scene_data_loader::scene_data_loader(const std::string& fpath)
        : pimpl_{std::make_unique<impl>(fpath)} {}

bool scene_data_loader::is_loaded() const { return pimpl_ != nullptr; }

void scene_data_loader::load(const std::string& f) {
    pimpl_ = std::make_unique<impl>(f);
}

void scene_data_loader::save(const std::string& f) const {
    if (is_loaded()) {
        pimpl_->save(f);
    } else {
        throw std::logic_error{"can't save if nothing's been loaded"};
    }
}

void scene_data_loader::clear() { pimpl_ = nullptr; }

const scene_data_loader::scene_data& scene_data_loader::get_scene_data() const {
    if (is_loaded()) {
        return pimpl_->get_scene_data();
    }
    throw std::logic_error{"can't access scene data if nothing's been loaded"};
}
}  // namespace core
}  // namespace wayverb
