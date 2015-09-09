#include "recursive_tetrahedral.h"
#include "scene_data.h"
#include "logger.h"

#include <gflags/gflags.h>

#include <iostream>

using namespace std;

int main(int argc, char ** argv) {
    Logger::restart();

    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Logger::log("parsed command line");

    if (argc != 2) {
        Logger::log_err("too few arguments");
        return EXIT_FAILURE;
    }

    Logger::log("continuing");

    CuboidBoundary cuboid_boundary(-1, 1);
    Logger::log("created cuboid");
    SphereBoundary sphere_boundary(0, 2);
    Logger::log("created sphere");
    SceneData scene_data(argv[1]);
    Logger::log("loaded scene data");
    auto mesh_boundary = scene_data.get_mesh_boundary();
    Logger::log("created mesh with boundaries:");

    Logger::log(mesh_boundary.boundary.c0, ", ", mesh_boundary.boundary.c1);

    auto mesh = tetrahedral_mesh(mesh_boundary, Vec3f(0, 2, 0), 0.2);

    Logger::log("created tetrahedral mesh within boundary");

    for (auto i : mesh)
        cout << i.position.x << " " << i.position.y << " " << i.position.z
             << " " << i.ports[0] << " " << i.ports[1] << " " << i.ports[2]
             << " " << i.ports[3] << endl;

    return 0;
}
