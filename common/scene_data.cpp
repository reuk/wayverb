#include "scene_data.h"

#include "conversions.h"

#include "config.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "rapidjson/document.h"

#include <vector>
#include <stdexcept>
#include <fstream>

using namespace std;
using namespace rapidjson;

void attemptJsonParse (const string & fname, Document & doc) {
    ifstream in (fname);
    string file
    (   (istreambuf_iterator <char> (in))
    ,   istreambuf_iterator <char>()
    );

    doc.Parse(file.c_str());
}

SurfaceLoader::SurfaceLoader(const string & fpath) {
    Surface defaultSurface {
        VolumeType({0.92, 0.92, 0.93, 0.93, 0.94, 0.95, 0.95, 0.95}),
        VolumeType({0.50, 0.90, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95})
    };

    add_surface("default", defaultSurface);

    Document document;
    attemptJsonParse (fpath, document);
    if (! document.IsObject())
        throw runtime_error ("Materials must be stored in a JSON object");

    for (auto i = document.MemberBegin(); i != document.MemberEnd(); ++i) {
        string name = i->name.GetString();

        Surface surface;
        ValueJsonValidator <Surface> (surface).run(i->value);

        add_surface(name, surface);
    }
}

void SurfaceLoader::add_surface(const string & name, const Surface & surface) {
    surfaces.push_back(surface);
    surface_indices[name] = surfaces.size() - 1;
}

std::vector<Surface> SurfaceLoader::get_surfaces() const {
    return surfaces;
}

SurfaceLoader::size_type SurfaceLoader::get_index(const std::string & name) const {
    auto ret = 0;
    auto it = surface_indices.find(name);
    if (it != surface_indices.end())
        ret = it->second;
    return ret;
}

SceneData::SceneData(const string & fpath, const string & mat_file) {
    populate(fpath, mat_file);
}

SceneData::SceneData(const aiScene * const scene, const string & mat_file) {
    populate(scene, mat_file);
}

void SceneData::populate(const aiScene * const scene, const string & mat_file) {
    if (!scene)
        throw runtime_error("scene pointer is null");

    SurfaceLoader surface_loader(mat_file);
    surfaces = surface_loader.get_surfaces();

    triangles.clear();
    vertices.clear();

    for (auto i = 0u; i != scene->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[i];

        auto material =
            scene->mMaterials[mesh->mMaterialIndex];
        aiString matName;
        material->Get(AI_MATKEY_NAME, matName);

        auto surface_index = surface_loader.get_index(matName.C_Str());

        vector<cl_float3> meshVertices(mesh->mNumVertices);

        for (auto j = 0u; j != mesh->mNumVertices; ++j) {
            meshVertices[j] = convert(mesh->mVertices[j]);
        }

        vector<Triangle> meshTriangles(mesh->mNumFaces);

        for (auto j = 0u; j != mesh->mNumFaces; ++j) {
            auto face = mesh->mFaces[j];

            meshTriangles[j] =
                Triangle{surface_index,
                    face.mIndices[0] + vertices.size(), face.mIndices[1] + vertices.size(), face.mIndices[2] + vertices.size()};
        }

        vertices.insert(
            vertices.end(), meshVertices.begin(), meshVertices.end());
        triangles.insert(
            triangles.end(), meshTriangles.begin(), meshTriangles.end());
    }
}

void SceneData::populate(const string & fpath, const string & mat_file) {
    Assimp::Importer importer;
    try {
        populate(importer.ReadFile(
            fpath,
            (aiProcess_Triangulate | aiProcess_GenSmoothNormals |
             aiProcess_FlipUVs)), mat_file);
    } catch (...) {
        throw runtime_error("failed to read file");
    }
}
