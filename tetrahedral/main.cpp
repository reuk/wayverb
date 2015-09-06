#include "tetrahedral.h"
#include "scene_data.h"

#include <iostream>

using namespace std;

int main() {
    CuboidBoundary cuboid_boundary(-1, 1);
    SphereBoundary sphere_boundary(0, 2);
    auto mesh_boundary = SceneData("test_scene.obj").get_mesh_boundary();

    auto mesh = tetrahedral_mesh(mesh_boundary, Vec3f(0), 0.1);

    for (auto i : mesh)
        cout << i.position.x << " " << i.position.y << " " << i.position.z << " " << i.ports[0] << " " << i.ports[1] << " " << i.ports[2] << " " << i.ports[3] << endl;

    return 0;
}
