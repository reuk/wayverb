#include "data.h"

#include "mesh_impulse_response.cpp"

const float * mesh_impulse_response::data = ::data;
const size_t mesh_impulse_response::size = sizeof(::data) / sizeof(float);