#include "common/scene_data.h"

#include "common/cl/scene_structs.h"
#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/range.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"

#include "common/serialize/json_read_write.h"

#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <fstream>
#include <stdexcept>
#include <vector>

aligned::vector<glm::vec3> convert(const aligned::vector<cl_float3>& c) {
    return map_to_vector(c, [](const auto& i) { return to_vec3(i); });
}

//----------------------------------------------------------------------------//

copyable_scene_data::copyable_scene_data(
        const aligned::vector<triangle>& triangles,
        const aligned::vector<cl_float3>& vertices,
        const aligned::vector<material>& materials)
        : contents{triangles, vertices, materials} {}

copyable_scene_data::copyable_scene_data(struct contents&& rhs)
        : contents(std::move(rhs)) {}

geo::box copyable_scene_data::get_aabb() const {
    const auto v = convert(get_vertices());
    return util::min_max(std::begin(v), std::end(v));
}

aligned::vector<size_t> copyable_scene_data::compute_triangle_indices() const {
    aligned::vector<size_t> ret(get_triangles().size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<triangle>& copyable_scene_data::get_triangles() const {
    return contents.triangles;
}
const aligned::vector<cl_float3>& copyable_scene_data::get_vertices() const {
    return contents.vertices;
}
const aligned::vector<scene_data::material>&
copyable_scene_data::get_materials() const {
    return contents.materials;
}

aligned::vector<surface> copyable_scene_data::get_surfaces() const {
    aligned::vector<surface> ret(get_materials().size());
    proc::transform(get_materials(), ret.begin(), [](const auto& i) {
        return i.surface;
    });
    return ret;
}

void copyable_scene_data::set_surfaces(
        const aligned::vector<material>& materials) {
    for (const auto& i : materials) {
        set_surface(i);
    }
}

void copyable_scene_data::set_surfaces(
        const aligned::map<std::string, surface>& surfaces) {
    for (auto& i : surfaces) {
        set_surface(material{i.first, i.second});
    }
}

void copyable_scene_data::set_surface(const material& material) {
    auto it = proc::find_if(contents.materials, [&material](const auto& i) {
        return i.name == material.name;
    });
    if (it != get_materials().end()) {
        it->surface = material.surface;
    }
}

void copyable_scene_data::set_surfaces(const surface& surface) {
    proc::for_each(contents.materials,
                   [surface](auto& i) { i.surface = surface; });
}

//----------------------------------------------------------------------------//

struct scene_data::impl {
    Assimp::Importer importer;
};

scene_data::scene_data(scene_data&& rhs) noexcept = default;
scene_data& scene_data::operator=(scene_data&& rhs) noexcept = default;
scene_data::~scene_data() noexcept                           = default;

scene_data::scene_data(const std::string& scene_file)
        : scene_data(load(scene_file)) {}

scene_data::scene_data(copyable_scene_data&& rhs, std::unique_ptr<impl>&& pimpl)
        : copyable_scene_data(std::move(rhs))
        , pimpl(std::move(pimpl)) {}

scene_data::scene_data(
        std::tuple<copyable_scene_data, std::unique_ptr<impl>>&& rhs)
        : scene_data(std::move(std::get<0>(rhs)), std::move(std::get<1>(rhs))) {
}

std::tuple<copyable_scene_data, std::unique_ptr<scene_data::impl>>
scene_data::load(const std::string& scene_file) {
    auto pimpl = std::make_unique<impl>();
    auto scene = pimpl->importer.ReadFile(
            scene_file,
            (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
             aiProcess_FlipUVs));

    if (!scene) {
        throw std::runtime_error("scene pointer is null");
    }

    struct copyable_scene_data::contents contents;
    contents.materials.resize(scene->mNumMaterials);
    std::transform(scene->mMaterials,
                   scene->mMaterials + scene->mNumMaterials,
                   contents.materials.begin(),
                   [](auto i) {
                       aiString material_name;
                       i->Get(AI_MATKEY_NAME, material_name);
                       return material{material_name.C_Str(), surface{}};
                   });

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        aligned::vector<cl_float3> vertices(mesh->mNumVertices);
        std::transform(mesh->mVertices,
                       mesh->mVertices + mesh->mNumVertices,
                       vertices.begin(),
                       [](auto i) { return to_cl_float3(i); });

        aligned::vector<triangle> triangles(mesh->mNumFaces);
        std::transform(mesh->mFaces,
                       mesh->mFaces + mesh->mNumFaces,
                       triangles.begin(),
                       [&mesh, &contents](auto i) {
                           return triangle{
                                   mesh->mMaterialIndex,
                                   i.mIndices[0] + contents.vertices.size(),
                                   i.mIndices[1] + contents.vertices.size(),
                                   i.mIndices[2] + contents.vertices.size()};
                       });

        contents.vertices.insert(
                contents.vertices.end(), vertices.begin(), vertices.end());
        contents.triangles.insert(
                contents.triangles.end(), triangles.begin(), triangles.end());
    }

    // proc::for_each(contents.vertices, [](auto& i) {
    //    std::for_each(std::begin(i.s), std::end(i.s), [scale](auto& i) {
    //        i *= scale;
    //    });
    //});

    return std::make_tuple(copyable_scene_data(std::move(contents)),
                           std::move(pimpl));
}

void scene_data::save(const std::string& f) const {
    Assimp::Exporter().Export(pimpl->importer.GetScene(), "obj", f);
}
