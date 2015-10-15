#pragma once

#include "vec.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <vector>

using Triangle = Vec3<uint32_t>;

class SceneData {
public:
    SceneData(const std::string & fpath = "");
    SceneData(const aiScene * const scene);
    virtual ~SceneData() noexcept = default;
    void populate(const aiScene * const scene);
    void populate(const std::string & fpath);

    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
};
