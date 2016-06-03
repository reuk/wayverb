#pragma once

#include "BasicDrawableObject.hpp"
#include "BoxObject.hpp"

#include "common/octree.h"
#include "common/scene_data.h"

class ModelSectionObject final : public BasicDrawableObject {
public:
    ModelSectionObject(const GenericShader& shader,
                       const SceneData& scene_data,
                       const Octree& octree);
    void draw() const override;

private:
    class DrawableOctree final : public ::Drawable {
    public:
        DrawableOctree(const GenericShader& shader, const Octree& octree);
        void draw() const override;

    private:
        void draw_worker(BoxObject& box) const;
        const GenericShader& shader;

        bool do_draw;
        CuboidBoundary aabb;
        std::vector<DrawableOctree> nodes;
    };

    std::vector<GLuint> get_indices(const SceneData& scene_data,
                                    const Octree& octree) const;

    DrawableOctree octree;
};
