#pragma once

#include "cl_structs.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <vector>

class SceneData {
public:
    SceneData(const std::string & fpath = "");
    SceneData(const aiScene * const scene);
    void populate(const aiScene * const scene);
    void populate(const std::string & fpath);
    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
};
