#pragma once

#include "core/cl/scene_structs.h"
#include "core/scene_data.h"

namespace wayverb {
namespace core {

using gpu_scene_data = generic_scene_data<cl_float3, surface<simulation_bands>>;

}  // namespace core
}  // namespace wayverb
