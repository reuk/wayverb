#include "core/scene_data_loader.h"
#include "core/conversions.h"
#include "core/scene_data.h"

#include "utilities/map_to_vector.h"
#include "utilities/string_builder.h"

#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace wayverb {
namespace core {

class scene_data_loader::impl final {
    auto load_from_file(const std::string& scene_file) {
        const auto scene = importer_.ReadFile(
                scene_file,
                (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                 aiProcess_FlipUVs));

        if (scene == nullptr) {
            throw std::runtime_error{util::build_string(
                    "Couldn't load scene.\n", importer_.GetErrorString())};
        }

        util::aligned::vector<triangle> triangles{};
        util::aligned::vector<cl_float3> vertices{};

        auto materials = util::map_to_vector(
                scene->mMaterials,
                scene->mMaterials + scene->mNumMaterials,
                [](auto i) {
                    aiString material_name;
                    i->Get(AI_MATKEY_NAME, material_name);
                    return std::string{material_name.C_Str()};
                });

        for (auto i = 0u; i != scene->mNumMeshes; ++i) {
            const auto mesh = scene->mMeshes[i];

            auto mesh_vertices =
                    util::map_to_vector(mesh->mVertices,
                                        mesh->mVertices + mesh->mNumVertices,
                                        to_cl_float3{});
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

        if (triangles.empty() || vertices.empty()) {
            throw std::runtime_error{"no geometry found in scene file"};
        }

        return make_scene_data(std::move(triangles),
                               std::move(vertices),
                               std::move(materials));
    }

public:
    impl() = default;

    impl(const std::string& f) { load(f); }

    void load(const std::string& f) { data_ = load_from_file(f); }

    void save(const std::string& f) const {
        if (data_) {
            Assimp::Exporter().Export(importer_.GetScene(), "obj", f);
        }
    }

    const std::experimental::optional<scene_data>& get_scene_data() const {
        return data_;
    }

    void clear() { data_ = std::experimental::nullopt; };

    std::string get_extensions() const {
        aiString str;
        importer_.GetExtensionList(str);
        return str.C_Str();
    }

private:
    Assimp::Importer importer_;
    std::experimental::optional<scene_data> data_;
};

////////////////////////////////////////////////////////////////////////////////

scene_data_loader::scene_data_loader()
        : pimpl_{std::make_unique<impl>()} {}

scene_data_loader::scene_data_loader(scene_data_loader&&) noexcept = default;
scene_data_loader& scene_data_loader::operator=(scene_data_loader&&) noexcept =
        default;
scene_data_loader::~scene_data_loader() noexcept = default;

scene_data_loader::scene_data_loader(const std::string& fpath)
        : pimpl_{std::make_unique<impl>(fpath)} {}

void scene_data_loader::load(const std::string& f) { pimpl_->load(f); }

void scene_data_loader::save(const std::string& f) const { pimpl_->save(f); }

void scene_data_loader::clear() { pimpl_->clear(); }

std::string scene_data_loader::get_extensions() const {
    return pimpl_->get_extensions();
}

const std::experimental::optional<scene_data_loader::scene_data>&
scene_data_loader::get_scene_data() const {
    return pimpl_->get_scene_data();
}
}  // namespace core
}  // namespace wayverb
