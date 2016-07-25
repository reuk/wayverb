#include "common/scene_data.h"

#include "common/boundaries.h"
#include "common/conversions.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/triangle.h"

#include "common/serialize/json_read_write.h"

#include "assimp/Exporter.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <glog/logging.h>

#include <fstream>
#include <stdexcept>
#include <vector>

//----------------------------------------------------------------------------//

CopyableSceneData::CopyableSceneData(const aligned::vector<Triangle>& triangles,
                                     const aligned::vector<cl_float3>& vertices,
                                     const aligned::vector<Material>& materials)
        : contents{triangles, vertices, materials} {}

CopyableSceneData::CopyableSceneData(Contents&& rhs)
        : contents(std::move(rhs)) {}

CuboidBoundary CopyableSceneData::get_aabb() const {
    return CuboidBoundary(min_max(get_converted_vertices()));
}

aligned::vector<glm::vec3> CopyableSceneData::get_converted_vertices() const {
    aligned::vector<glm::vec3> vec(get_vertices().size());
    proc::transform(
            get_vertices(), vec.begin(), [](auto i) { return to_vec3f(i); });
    return vec;
}

aligned::vector<int> CopyableSceneData::get_triangle_indices() const {
    aligned::vector<int> ret(get_triangles().size());
    proc::iota(ret, 0);
    return ret;
}

const aligned::vector<Triangle>& CopyableSceneData::get_triangles() const {
    return contents.triangles;
}
const aligned::vector<cl_float3>& CopyableSceneData::get_vertices() const {
    return contents.vertices;
}
const aligned::vector<SceneData::Material>& CopyableSceneData::get_materials()
        const {
    return contents.materials;
}

aligned::vector<Surface> CopyableSceneData::get_surfaces() const {
    aligned::vector<Surface> ret(get_materials().size());
    proc::transform(get_materials(), ret.begin(), [](const auto& i) {
        return i.surface;
    });
    return ret;
}

void CopyableSceneData::set_surfaces(
        const aligned::vector<Material>& materials) {
    for (const auto& i : materials) {
        set_surface(i);
    }
}

void CopyableSceneData::set_surfaces(
        const aligned::map<std::string, Surface>& surfaces) {
    for (auto& i : surfaces) {
        set_surface(Material{i.first, i.second});
    }
}

void CopyableSceneData::set_surface(const Material& material) {
    auto it = proc::find_if(contents.materials, [&material](const auto& i) {
        return i.name == material.name;
    });
    if (it != get_materials().end()) {
        it->surface = material.surface;
    }
}

void CopyableSceneData::set_surfaces(const Surface& surface) {
    proc::for_each(contents.materials,
                   [surface](auto& i) { i.surface = surface; });
}

//----------------------------------------------------------------------------//

struct SceneData::Impl {
    Assimp::Importer importer;
};

SceneData::SceneData(SceneData&& rhs) noexcept = default;
SceneData& SceneData::operator=(SceneData&& rhs) noexcept = default;
SceneData::~SceneData() noexcept                          = default;

SceneData::SceneData(const std::string& scene_file)
        : SceneData(load(scene_file)) {}

SceneData::SceneData(CopyableSceneData&& rhs, std::unique_ptr<Impl>&& pimpl)
        : CopyableSceneData(std::move(rhs))
        , pimpl(std::move(pimpl)) {}

SceneData::SceneData(std::tuple<CopyableSceneData, std::unique_ptr<Impl>>&& rhs)
        : SceneData(std::move(std::get<0>(rhs)), std::move(std::get<1>(rhs))) {}

std::tuple<CopyableSceneData, std::unique_ptr<SceneData::Impl>> SceneData::load(
        const std::string& scene_file) {
    auto impl  = std::make_unique<Impl>();
    auto scene = impl->importer.ReadFile(
            scene_file,
            (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
             aiProcess_FlipUVs));

    if (!scene) {
        throw std::runtime_error("scene pointer is null");
    }

    CopyableSceneData::Contents contents;
    contents.materials.resize(scene->mNumMaterials);
    std::transform(scene->mMaterials,
                   scene->mMaterials + scene->mNumMaterials,
                   contents.materials.begin(),
                   [](auto i) {
                       aiString material_name;
                       i->Get(AI_MATKEY_NAME, material_name);
                       return Material{material_name.C_Str(), Surface{}};
                   });

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        aligned::vector<cl_float3> vertices(mesh->mNumVertices);
        std::transform(mesh->mVertices,
                       mesh->mVertices + mesh->mNumVertices,
                       vertices.begin(),
                       [](auto i) { return to_cl_float3(i); });

        aligned::vector<Triangle> triangles(mesh->mNumFaces);
        std::transform(mesh->mFaces,
                       mesh->mFaces + mesh->mNumFaces,
                       triangles.begin(),
                       [&mesh, &contents](auto i) {
                           return Triangle{
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

    return std::make_tuple(CopyableSceneData(std::move(contents)),
                           std::move(impl));
}

void SceneData::save(const std::string& f) const {
    Assimp::Exporter().Export(pimpl->importer.GetScene(), "obj", f);
}
