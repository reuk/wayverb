#include "tetrahedral.h"

#include <iostream>

using namespace std;

int main() {
    CuboidBoundary cuboid_boundary(-1, 1);
    SphereBoundary sphere_boundary(0, 2);
    auto mesh = tetrahedral_mesh(sphere_boundary, 0, 0.1);

    for (auto i : mesh)
        cout << i.position.x << " " << i.position.y << " " << i.position.z << " " << i.ports[0] << " " << i.ports[1] << " " << i.ports[2] << " " << i.ports[3] << endl;

    return 0;
}
