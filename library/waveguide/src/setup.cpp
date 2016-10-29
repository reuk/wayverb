#include "waveguide/setup.h"
#include "waveguide/mesh_setup_program.h"

namespace waveguide {

vectors::vectors(aligned::vector<condensed_node> nodes,
                 aligned::vector<coefficients_canonical> coefficients,
                 boundary_index_data boundary_index_data)
        : condensed_nodes_(std::move(nodes))
        , coefficients_(std::move(coefficients))
        , boundary_index_data_(std::move(boundary_index_data)) {
#ifndef NDEBUG
    auto throw_if_mismatch = [&](auto checker, auto size) {
        if (count_boundary_type(condensed_nodes_.begin(),
                                condensed_nodes_.end(),
                                checker) != size) {
            throw std::runtime_error(
                    "number of nodes does not match number of boundaries");
        }
    };

    throw_if_mismatch(is_boundary<1>, boundary_index_data_.b1.size());
    throw_if_mismatch(is_boundary<2>, boundary_index_data_.b2.size());
    throw_if_mismatch(is_boundary<3>, boundary_index_data_.b3.size());
#endif
}

const aligned::vector<condensed_node>& vectors::get_condensed_nodes() const {
    return condensed_nodes_;
}

const aligned::vector<coefficients_canonical>& vectors::get_coefficients()
        const {
    return coefficients_;
}

void vectors::set_coefficients(coefficients_canonical c) {
    std::fill(begin(coefficients_), end(coefficients_), c);
}

void vectors::set_coefficients(aligned::vector<coefficients_canonical> c) {
    if (c.size() != coefficients_.size()) {
        throw std::runtime_error(
                "size of new coefficients vector must be equal to the existing "
                "one in order to maintain object invariants");
    }
    coefficients_ = std::move(c);
}

}  // namespace waveguide
