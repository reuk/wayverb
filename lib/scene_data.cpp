#include "scene_data.h"

#include "conversions.h"

#include <vector>
#include <stdexcept>

using namespace std;

inline cl_float3 fromAIVec(const aiVector3D & v) {
    return (cl_float3){{v.x, v.y, v.z, 0}};
}

SceneData::SceneData(const string & fpath) {
    populate(fpath);
}

SceneData::SceneData(const aiScene * const scene) {
    populate(scene);
}

void SceneData::populate(const aiScene * const scene) {
    if (!scene)
        throw runtime_error("scene pointer is null");

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        vector<cl_float3> meshVertices(mesh->mNumVertices);

        for (auto j = 0u; j != mesh->mNumVertices; ++j) {
            meshVertices[j] = fromAIVec(mesh->mVertices[j]);
        }

        vector<Triangle> meshTriangles(mesh->mNumFaces);

        for (auto j = 0u; j != mesh->mNumFaces; ++j) {
            auto face = mesh->mFaces[j];

            meshTriangles[j] =
                Vec3<uint32_t>(
                    face.mIndices[0], face.mIndices[1], face.mIndices[2]) +
                vertices.size();
        }

        vertices.insert(
            vertices.end(), meshVertices.begin(), meshVertices.end());
        triangles.insert(
            triangles.end(), meshTriangles.begin(), meshTriangles.end());
    }
}

void SceneData::populate(const string & fpath) {
    Assimp::Importer importer;
    try {
        populate(importer.ReadFile(
            fpath,
            (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
             aiProcess_FlipUVs)));
    } catch (...) {
        throw runtime_error("failed to read file");
    }
}

MeshBoundary SceneData::get_mesh_boundary() const {
    vector<Vec3f> v(vertices.size());
    transform(vertices.begin(),
              vertices.end(),
              v.begin(),
              [](auto i) { return convert(i); });
    return MeshBoundary(triangles, v);
}
