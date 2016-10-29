#pragma once

#include "waveguide/boundary_coefficient_finder.h"
#include "waveguide/cl/utils.h"
#include "waveguide/mesh_descriptor.h"
#include "waveguide/program.h"

#include "common/cl/include.h"
#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxel_collection.h"

#include "utilities/aligned/vector.h"
#include "utilities/map_to_vector.h"

#include "glm/fwd.hpp"

namespace waveguide {

constexpr bool is_inside(const condensed_node& c) {
    return c.boundary_type && id_inside;
}

////////////////////////////////////////////////////////////////////////////////

class vectors final {
public:
    vectors(aligned::vector<condensed_node> nodes,
            aligned::vector<coefficients_canonical>
                    coefficients,
            boundary_index_data boundary_index_data);

    template <size_t n>
    const aligned::vector<boundary_index_array<n>>& get_boundary_indices()
            const;

    const aligned::vector<condensed_node>& get_condensed_nodes() const;
    const aligned::vector<coefficients_canonical>& get_coefficients() const;

    void set_coefficients(coefficients_canonical coefficients);
    void set_coefficients(aligned::vector<coefficients_canonical> coefficients);

private:
    aligned::vector<condensed_node> condensed_nodes_;
    aligned::vector<coefficients_canonical> coefficients_;
    boundary_index_data boundary_index_data_;
};

template <>
inline const aligned::vector<boundary_index_array<1>>&
vectors::get_boundary_indices<1>() const {
    return boundary_index_data_.b1;
}
template <>
inline const aligned::vector<boundary_index_array<2>>&
vectors::get_boundary_indices<2>() const {
    return boundary_index_data_.b2;
}
template <>
inline const aligned::vector<boundary_index_array<3>>&
vectors::get_boundary_indices<3>() const {
    return boundary_index_data_.b3;
}

////////////////////////////////////////////////////////////////////////////////

template <size_t N>
static boundary_data_array<N> construct_boundary_data_array(
        const boundary_index_array<N>& arr) {
    boundary_data_array<N> ret{};
    for (auto i = 0u; i != N; ++i) {
        ret.array[i].coefficient_index = arr.array[i];
    }
    return ret;
}

template <size_t n>
inline aligned::vector<boundary_data_array<n>> get_boundary_data(
        const vectors& d) {
    const auto indices = d.get_boundary_indices<n>();
    return map_to_vector(begin(indices), end(indices), [](const auto& i) {
        return construct_boundary_data_array(i);
    });
}

}  // namespace waveguide
