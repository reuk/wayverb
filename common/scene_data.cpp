#include "scene_data.h"

#include "boundaries.h"
#include "conversions.h"
#include "json_read_write.h"
#include "stl_wrappers.h"
#include "string_builder.h"
#include "triangle.h"

#include "surface_owner_serialize.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <cereal/types/map.hpp>

#include <glog/logging.h>

#include <fstream>
#include <stdexcept>
#include <vector>

SurfaceLoader::SurfaceLoader(const std::string& path) {
    std::map<std::string, Surface> loaded;

    try {
        json_read_write::read(path, cereal::make_nvp("surfaces", loaded));
    } catch (const cereal::RapidJSONException& e) {
        std::cout << e.what() << std::endl;
    }

    add_surface(
        "default",
        Surface{VolumeType{{0.92, 0.92, 0.93, 0.93, 0.94, 0.95, 0.95, 0.95}},
                VolumeType{{0.50, 0.90, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95}}});

    for (const auto& i : loaded) {
        add_surface(i.first, i.second);
    }
}

void SurfaceLoader::add_surface(const std::string& name,
                                const Surface& surface) {
    if (surface_indices.find(name) == surface_indices.end()) {
        surfaces.push_back(surface);
        surface_indices[name] = surfaces.size() - 1;
    } else {
        LOG(INFO) << "tried to add duplicate surface";
    }
}

const std::vector<Surface>& SurfaceLoader::get_surfaces() const {
    return surfaces;
}

SurfaceLoader::size_type SurfaceLoader::get_index(
    const std::string& name) const {
    auto ret = 0;
    auto it = surface_indices.find(name);
    if (it != surface_indices.end()) {
        ret = it->second;
    }
    return ret;
}

SurfaceOwner::SurfaceOwner(const std::vector<Surface>& surfaces)
        : surfaces(surfaces) {
    check_num_surfaces();
}
SurfaceOwner::SurfaceOwner(std::vector<Surface>&& surfaces)
        : surfaces(std::move(surfaces)) {
    check_num_surfaces();
}
SurfaceOwner::SurfaceOwner(const SurfaceLoader& surface_loader)
        : surfaces(surface_loader.surfaces) {
    check_num_surfaces();
}
SurfaceOwner::SurfaceOwner(SurfaceLoader&& surface_loader)
        : surfaces(std::move(surface_loader.surfaces)) {
    check_num_surfaces();
}

void SurfaceOwner::check_num_surfaces() const {
    if (surfaces.empty()) {
        throw std::runtime_error("must own at least one surface");
    }
}

const std::vector<Surface>& SurfaceOwner::get_surfaces() const {
    return surfaces;
}

SceneData::SceneData(const Contents& contents)
        : SceneData(contents.triangles, contents.vertices, contents.surfaces) {
}
SceneData::SceneData(Contents&& contents)
        : SceneData(std::move(contents.triangles),
                    std::move(contents.vertices),
                    std::move(contents.surfaces)) {
}

SceneData::SceneData(const std::vector<Triangle>& triangles,
                     const std::vector<cl_float3>& vertices,
                     const std::vector<Surface>& surfaces)
        : SurfaceOwner(surfaces)
        , triangles(triangles)
        , vertices(vertices) {
}

SceneData::SceneData(std::vector<Triangle>&& triangles,
                     std::vector<cl_float3>&& vertices,
                     std::vector<Surface>&& surfaces)
        : SurfaceOwner(std::move(surfaces))
        , triangles(std::move(triangles))
        , vertices(std::move(vertices)) {
}

SceneData::SceneData(const std::string& fpath,
                     const std::string& mat_file,
                     float scale)
        : SceneData(Assimp::Importer().ReadFile(
                        fpath,
                        (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                         aiProcess_FlipUVs)),
                    mat_file,
                    scale) {
}

SceneData::SceneData(const aiScene* const scene,
                     const std::string& mat_file,
                     float scale)
        : SceneData(scene, SurfaceLoader(mat_file), scale) {
}

SceneData::SceneData(const aiScene* const scene,
                     const SurfaceLoader& loader,
                     float scale)
        : SceneData(get_contents(scene, loader, scale)) {
}

SceneData::Contents SceneData::get_contents(const aiScene* const scene,
                                            const SurfaceLoader& loader,
                                            float scale) {
    if (!scene) {
        throw std::runtime_error("scene pointer is null");
    }

    Contents ret;
    ret.surfaces = loader.get_surfaces();

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        auto material = scene->mMaterials[mesh->mMaterialIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);

        auto surface_index = loader.get_index(matName.C_Str());

        std::vector<cl_float3> meshVertices(mesh->mNumVertices);

        for (auto j = 0u; j != mesh->mNumVertices; ++j) {
            meshVertices[j] = to_cl_float3(mesh->mVertices[j]);
        }

        std::vector<Triangle> meshTriangles(mesh->mNumFaces);

        for (auto j = 0u; j != mesh->mNumFaces; ++j) {
            auto face = mesh->mFaces[j];

            meshTriangles[j] = Triangle{surface_index,
                                        face.mIndices[0] + ret.vertices.size(),
                                        face.mIndices[1] + ret.vertices.size(),
                                        face.mIndices[2] + ret.vertices.size()};
        }

        ret.vertices.insert(
            ret.vertices.end(), meshVertices.begin(), meshVertices.end());
        ret.triangles.insert(
            ret.triangles.end(), meshTriangles.begin(), meshTriangles.end());
    }

    proc::for_each(ret.vertices, [scale](auto& i) {
        std::for_each(
            std::begin(i.s), std::end(i.s), [scale](auto& i) { i *= scale; });
    });
    return ret;
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
