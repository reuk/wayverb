#include "common/scene_data.h"

#include "common/boundaries.h"
#include "common/conversions.h"
#include "common/json_read_write.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/surface_serialize.h"
#include "common/triangle.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <glog/logging.h>

#include <fstream>
#include <stdexcept>
#include <vector>

SurfaceConfig::SurfaceConfig(const std::map<std::string, Surface>& surfaces)
        : surfaces(surfaces) {
}

SurfaceConfig::SurfaceConfig(std::istream& stream) {
    load(stream);
}

SurfaceConfig::SurfaceConfig(const std::string& path) {
    load(path);
}

void SurfaceConfig::load(const std::string& fpath) {
    std::ifstream stream(fpath);
    load(stream);
}

void SurfaceConfig::load(std::istream& stream) {
    cereal::JSONInputArchive archive(stream);
    serialize(archive);
}

void SurfaceConfig::save(const std::string& fpath) {
    std::ofstream stream(fpath);
    save(stream);
}

void SurfaceConfig::save(std::ostream& stream) {
    cereal::JSONOutputArchive archive(stream);
    serialize(archive);
}

const std::map<std::string, Surface>& SurfaceConfig::get_surfaces() const {
    return surfaces;
}

CuboidBoundary SceneData::get_aabb() const {
    return CuboidBoundary(get_surrounding_box(get_converted_vertices()));
}

std::vector<Vec3f> SceneData::get_converted_vertices() const {
    std::vector<Vec3f> vec(vertices.size());
    proc::transform(vertices, vec.begin(), [](auto i) { return to_vec3f(i); });
    return vec;
}

std::vector<int> SceneData::get_triangle_indices() const {
    std::vector<int> ret(triangles.size());
    proc::iota(ret, 0);
    return ret;
}

const std::vector<Triangle>& SceneData::get_triangles() const {
    return triangles;
}
const std::vector<cl_float3>& SceneData::get_vertices() const {
    return vertices;
}

const std::vector<SceneData::Material>& SceneData::get_materials() const {
    return materials;
}

SceneData::SceneData(const aiScene* const scene, float scale)
        : SceneData(load(scene, scale)) {
}

SceneData::SceneData(const std::string& fpath, float scale)
        : SceneData(Assimp::Importer().ReadFile(
                        fpath,
                        (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                         aiProcess_FlipUVs)),
                    scale) {
}

SceneData::SceneData(const std::vector<Triangle>& triangles,
                     const std::vector<cl_float3>& vertices,
                     const std::vector<Material>& materials)
        : triangles(triangles)
        , vertices(vertices)
        , materials(materials) {
}

SceneData::SceneData(const Contents& contents)
        : triangles(contents.triangles)
        , vertices(contents.vertices)
        , materials(contents.materials) {
}

SceneData::SceneData(const std::string& fpath,
                     const std::string& mat,
                     float scale)
        : SceneData(fpath, scale) {
    set_surfaces(SurfaceConfig(mat));
}

SceneData::Contents SceneData::load(const aiScene* const scene, float scale) {
    if (!scene) {
        throw std::runtime_error("scene pointer is null");
    }

    Contents ret;

    ret.materials.resize(scene->mNumMaterials);
    std::transform(scene->mMaterials,
                   scene->mMaterials + scene->mNumMaterials,
                   ret.materials.begin(),
                   [](auto i) {
                       aiString material_name;
                       i->Get(AI_MATKEY_NAME, material_name);
                       return Material{material_name.C_Str(), Surface{}};
                   });

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        std::vector<cl_float3> vertices(mesh->mNumVertices);
        std::transform(mesh->mVertices,
                       mesh->mVertices + mesh->mNumVertices,
                       vertices.begin(),
                       [](auto i) { return to_cl_float3(i); });

        std::vector<Triangle> triangles(mesh->mNumFaces);
        std::transform(mesh->mFaces,
                       mesh->mFaces + mesh->mNumFaces,
                       triangles.begin(),
                       [&mesh, &ret](auto i) {
                           return Triangle{mesh->mMaterialIndex,
                                           i.mIndices[0] + ret.vertices.size(),
                                           i.mIndices[1] + ret.vertices.size(),
                                           i.mIndices[2] + ret.vertices.size()};
                       });

        ret.vertices.insert(
            ret.vertices.end(), vertices.begin(), vertices.end());
        ret.triangles.insert(
            ret.triangles.end(), triangles.begin(), triangles.end());
    }

    proc::for_each(ret.vertices, [scale](auto& i) {
        std::for_each(
            std::begin(i.s), std::end(i.s), [scale](auto& i) { i *= scale; });
    });

    return ret;
}

std::vector<Surface> SceneData::get_surfaces() const {
    std::vector<Surface> ret(materials.size());
    proc::transform(
        materials, ret.begin(), [](const auto& i) { return i.surface; });
    return ret;
}

void SceneData::set_surfaces(const std::map<std::string, Surface>& surfaces) {
    proc::for_each(surfaces,
                   [this](const auto& i) { set_surface(i.first, i.second); });
}

void SceneData::set_surfaces(const SurfaceConfig& surfaces) {
    set_surfaces(surfaces.get_surfaces());
}

void SceneData::set_surface(const std::string& name, const Surface& surface) {
    auto it = proc::find_if(materials,
                            [&name](const auto& i) { return i.name == name; });
    if (it != materials.end()) {
        it->surface = surface;
    }
}

void SceneData::set_surfaces(const Surface& surface) {
    proc::for_each(materials, [surface](auto& i) { i.surface = surface; });
}
