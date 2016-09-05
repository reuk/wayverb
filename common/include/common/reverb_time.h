#pragma once

#include "common/aligned/set.h"
#include "common/scene_data.h"

/// IMPORTANT
/// The following functions assume that the scene being modelled has the
/// following properties:
///     All triangle normals are oriented in the same direction.
///         So, no pathological geometry in the scene (moebius strips etc.)
///     The scene consists of a single, closed cavity.
///         No gaps or openings, and no uncoupled spaces.
/// If these critera are not met, the equations are not guaranteed to return
/// a result, let alone a *valid* result.

/// Find the area covered by a particular material.
float area(const scene_data& scene, size_t material_index);

/// Find the total surface area of an object.
float area(const scene_data& scene);

/// The product of the area covered by a material with the absorption
/// coefficient of that material.
volume_type absorption_area(const scene_data& scene, size_t surface_index);

/// Sum of the absorption areas for all materials in the scene.
volume_type equivalent_absorption_area(const scene_data& scene);

//----------------------------------------------------------------------------//
//
std::array<std::pair<cl_uint, cl_uint>, 3> get_index_pairs(const triangle& t);

/// This probably isn't the fastest way of doing this...
template <typename It>  /// Iterator through a collection of triangles.
bool triangles_are_oriented(It begin, It end) {
    //  The scene is consistently oriented if no triangles in the scene share
    //  an edge with the same winding direction.
    aligned::set<std::pair<cl_uint, cl_uint>> table;

    //  For each triangle.
    for (; begin != end; ++begin) {
        //  For each pair of vertices.
        for (const auto& pair : get_index_pairs(*begin)) {
            //  See whether this pair of vertices has already been found.
            if (! table.insert(pair).second) {
                //  The pair already existed.
                return false;
            }
        }
    }
    return true;
}

float estimate_room_volume(const scene_data& scene);

//----------------------------------------------------------------------------//

//  USE THESE ONES

/// Sabine reverb time (use the damping constant function above too)
/// (kuttruff 5.9) (vorlander 4.33)
volume_type sabine_reverb_time(const scene_data& scene_data,
                               volume_type air_coefficient);

/// Eyring reverb time (kuttruff 5.24) (vorlander 4.32)
volume_type eyring_reverb_time(const scene_data& scene,
                               volume_type air_coefficient);
