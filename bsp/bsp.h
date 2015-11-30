#pragma once

#include "vec.h"

#include "scene_data.h"

class Plane
{
public:
    Plane(const Vec3f & normal = Vec3f(), float distance = 0);
    Vec3f normal;
    float distance;
};

class BspNode
{
    BspNode(const Plane & plane = Plane(), const std::vector<int> & polygons = std::vector<int>(), int front = -1, int back = -1);
    Plane plane;
    std::vector<int> polygons;
    int front;
    int back;
};

class BspTree
{
public:
    void build(const SceneData & scene_data);
private:
    std::vector<BspNode> nodes;
};
