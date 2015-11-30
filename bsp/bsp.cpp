#include "bsp.h"

Plane::Plane(const Vec3f & normal, float distance)
    : normal(normal), distance(distance)
{

}


BspNode::BspNode(const Plane & plane, const std::vector<int> & polygons, int front, int back)
    : plane(plane), polygons(polygons), front(front), back(back)
{

}

void BspTree::build(const SceneData & scene_data)
{

}
