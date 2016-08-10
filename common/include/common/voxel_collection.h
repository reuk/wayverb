#pragma once

#include "common/geo/rect.h"
#include "common/spatial_division.h"

/// This callback is used to check for intersections with the contents of
/// the collection.
/// The colleciton doesn't store any information about the triangles it
/// contains, so this callback is constructed with a reference to a
/// SceneData object which contains the triangle information.
class triangle_traversal_callback final {
public:
    triangle_traversal_callback(const copyable_scene_data& scene_data);
    geo::intersection operator()(const geo::ray& ray,
                                 const aligned::vector<size_t>& items) const;

private:
    aligned::vector<triangle> tri;
    aligned::vector<glm::vec3> vertices;
};

namespace detail {

//----------------------------------------------------------------------------//

using voxel = aligned::vector<size_t>;

//----------------------------------------------------------------------------//

template <size_t n>
struct voxel_data_trait final {
    using prev_type = voxel_data_trait<n - 1>;
    using nested_type = typename prev_type::data_type;
    using data_type = aligned::vector<nested_type>;
    static data_type get_blank(size_t size) {
        return data_type(size, prev_type::get_blank(size));
    }
};

template <>
struct voxel_data_trait<0> final {
    using data_type = voxel;
    static data_type get_blank(size_t) { return data_type{}; }
};

template <size_t n>
using voxel_data_t = typename voxel_data_trait<n>::data_type;

//----------------------------------------------------------------------------//

template <size_t n>
inline const voxel& index(const typename voxel_data_trait<n>::data_type& data,
                          indexing::index_t<n> i) {
    return index<n - 1>(data[indexing::back<n>(i)], indexing::reduce<n>(i));
}

template <>
inline const voxel& index<1>(
        const typename voxel_data_trait<1>::data_type& data,
        indexing::index_t<1> i) {
    return data[i];
}

template <size_t n>
inline voxel& index(typename voxel_data_trait<n>::data_type& data,
                    indexing::index_t<n> i) {
    return index<n - 1>(data[indexing::back<n>(i)], indexing::reduce<n>(i));
}

template <>
inline voxel& index<1>(typename voxel_data_trait<1>::data_type& data,
                       indexing::index_t<1> i) {
    return data[i];
}

//----------------------------------------------------------------------------//

template <size_t n>
void voxelise(const ndim_tree<n>& tree,
              const indexing::index_t<n>& position,
              voxel_data_t<n>& ret) {
    if (!tree.has_nodes()) {
        index<n>(ret, position) = tree.get_items();
    } else {
        for (size_t i = 0; i != tree.get_nodes().size(); ++i) {
            const auto relative = indexing::relative_position<n>(i) *
                                  static_cast<unsigned>(tree.get_side() / 2);
            voxelise(tree.get_nodes()[i], position + relative, ret);
        }
    }
}

template <size_t n>
voxel_data_t<n> voxelise(const ndim_tree<n>& tree) {
    auto ret = voxel_data_trait<n>::get_blank(tree.get_side());
    voxelise(tree, indexing::index_t<n>{0}, ret);
    return ret;
}

//----------------------------------------------------------------------------//

}  // namespace detail

/// A box full of voxels, where each voxel keeps track of its own boundary
/// and indices of triangles that overlap that boundary.
/// Can be 'flattened' - converts the collection into a memory-efficient
/// array representation, which can be passed to the GPU.
template <size_t n>
class voxel_collection final {
public:
    using aabb_type = util::detail::range_t<n>;
    using data_type = detail::voxel_data_t<n>;

    /// Construct directly from an existing tree.
    voxel_collection(const ndim_tree<n>& tree)
            : aabb(tree.get_aabb())
            , data(detail::voxelise(tree)) {}

    aabb_type get_aabb() const { return aabb; }
    size_t get_side() const { return data.size(); }
    const auto& get_voxel(indexing::index_t<n> i) const {
        return detail::index<n>(data, i);
    }
    auto& get_voxel(indexing::index_t<n> i) {
        return detail::index<n>(data, i);
    }

private:
    aabb_type aabb;
    data_type data;
};

//----------------------------------------------------------------------------//

template <size_t n>
auto voxel_dimensions(const voxel_collection<n>& voxels) {
    return dimensions(voxels.get_aabb()) /
           static_cast<float>(voxels.get_side());
}

template <size_t n, typename u>
auto voxel_aabb(const voxel_collection<n>& voxels, u i) {
    using vt = util::detail::range_t<n>;
    const auto dim = voxel_dimensions(voxels);
    const auto root = voxels.get_aabb().get_min() + (dim * i);
    return vt(root, root + dim);
}

/// Returns a flat array-representation of the collection.
/// TODO document the array format
aligned::vector<cl_uint> get_flattened(const voxel_collection<3>& voxels);

using traversal_callback = std::function<geo::intersection(
        const geo::ray&, const aligned::vector<size_t>&)>;

/// Find the closest object along a ray.
geo::intersection traverse(const voxel_collection<3>& voxels,
                           const geo::ray& ray,
                           const traversal_callback& fun);
