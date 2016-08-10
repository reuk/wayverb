#pragma once

namespace waveguide {

/// This program will set up a rectangular mesh, given a bunch of room geometry
/// and the number of nodes.

class mesh_setup_program {
public:
    typedef enum : cl_int {
        id_none = 0,
        id_inside = 1 << 0,
        id_nx = 1 << 1,
        id_px = 1 << 2,
        id_ny = 1 << 3,
        id_py = 1 << 4,
        id_nz = 1 << 5,
        id_pz = 1 << 6,
        id_reentrant = 1 << 7,
    } boundary_type;

    static constexpr boundary_type port_index_to_boundary_type(unsigned int i) {
        return static_cast<boundary_type>(1 << (i + 1));
    }

    static constexpr cl_uint no_neighbor{~cl_uint{0}};

    struct alignas(1 << 4) node final {
        static constexpr size_t num_ports{6};

        cl_uint ports[num_ports]{};  //  the indices of adjacent ports
        cl_float3 position{};        //  spatial position
        cl_char inside{};            //  is the node an air node?
        cl_int boundary_type{};      //  describes the boundary type
        cl_uint boundary_index{};  //  an index into a boundary descriptor array
    };

private:
};

inline bool operator==(const mesh_setup_program::node& a,
                       const mesh_setup_program::node& b) {
    return proc::equal(a.ports, std::begin(b.ports)) &&
           std::tie(a.position, a.inside, a.boundary_type, a.boundary_index) ==
                   std::tie(b.position,
                            b.inside,
                            b.boundary_type,
                            b.boundary_index);
}

inline bool operator!=(const mesh_setup_program::node& a,
                       const mesh_setup_program::node& b) {
    return !(a == b);
}

}  // namespace waveguide
