#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"

#include <vector>

std::vector<Node> tetrahedral_mesh(const Boundary & boundary,
                                   Vec3f start,
                                   float spacing);