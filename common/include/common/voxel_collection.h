#pragma once

#include "spatial_division.h"

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
template <size_t dimensions>
class voxel final {
public:
    explicit voxel(
            const box<dimensions>& aabb = box<dimensions>(),
            const aligned::vector<size_t>& items = aligned::vector<size_t>())
            : aabb(aabb)
            , items(items) {}

    voxel(const ndim_tree<dimensions>& o)
            : voxel(o.get_aabb(), o.get_items()) {}

    box<dimensions> get_aabb() const { return aabb; }
    const aligned::vector<size_t>& get_items() const { return items; }

private:
    box<dimensions> aabb;
    aligned::vector<size_t> items;
};

template <size_t>
class voxel_collection_data;

template <>
class voxel_collection_data<2> {
public:
    using type = aligned::vector<aligned::vector<voxel<2>>>;

    voxel_collection_data(const ndim_tree<2>& tree)
            : data(compute_data(tree)) {}

    static type get_blank(size_t side) {
        return type(side, aligned::vector<voxel<2>>(side));
    }

    const detail::voxel<2>& get_voxel(const glm::ivec2& i) const {
        return data[i.x][i.y];
    }

    box<2> get_voxel_aabb() const {
        return data.front().front().get_aabb();
    }

    const type& get_data() const { return data; }

protected:
    ~voxel_collection_data() noexcept = default;

private:
    static void compute_data(type& ret,
                             const ndim_tree<2>& tree,
                             const glm::ivec2& d) {
        if (!tree.has_nodes()) {
            ret[d.x][d.y] = detail::voxel<2>(tree);
        } else {
            auto count = 0u;
            for (const auto& i : tree.get_nodes()) {
                const auto x = (count & 1u) ? 1u : 0u;
                const auto y = (count & 2u) ? 1u : 0u;
                compute_data(ret,
                             i,
                             d +
                                     glm::ivec2(x, y) *
                                             static_cast<int>(tree.get_side()) /
                                             2);
                count += 1;
            }
        }
    }

    static type compute_data(const ndim_tree<2>& tree) {
        auto ret = detail::voxel_collection_data<2>::get_blank(tree.get_side());
        compute_data(ret, tree, glm::ivec2(0));
        return ret;
    }

    type data;
};

template <>
class voxel_collection_data<3> {
public:
    using type = aligned::vector<aligned::vector<aligned::vector<voxel<3>>>>;

    voxel_collection_data(const ndim_tree<3>& tree)
            : data(compute_data(tree)) {}

    static type get_blank(size_t side) {
        return type(side,
                    aligned::vector<aligned::vector<voxel<3>>>(
                            side, aligned::vector<voxel<3>>(side)));
    }

    box<3> get_voxel_aabb() const {
        return data.front().front().front().get_aabb();
    }

    const detail::voxel<3>& get_voxel(const glm::ivec3& i) const {
        return data[i.x][i.y][i.z];
    }

    const type& get_data() const { return data; }

protected:
    ~voxel_collection_data() noexcept = default;

private:
    static void compute_data(type& ret,
                             const ndim_tree<3>& tree,
                             const glm::ivec3& d) {
        if (!tree.has_nodes()) {
            ret[d.x][d.y][d.z] = detail::voxel<3>(tree);
        } else {
            auto count = 0u;
            for (const auto& i : tree.get_nodes()) {
                const auto x = (count & 1u) ? 1u : 0u;
                const auto y = (count & 2u) ? 1u : 0u;
                const auto z = (count & 4u) ? 1u : 0u;
                compute_data(ret,
                             i,
                             d +
                                     glm::ivec3(x, y, z) *
                                             static_cast<int>(tree.get_side()) /
                                             2);
                count += 1;
            }
        }
    }

    static type compute_data(const ndim_tree<3>& tree) {
        auto ret = detail::voxel_collection_data<3>::get_blank(tree.get_side());
        compute_data(ret, tree, glm::ivec3(0));
        return ret;
    }

    type data;
};

template <size_t n>
using voxel_collection_data_t = typename voxel_collection_data<n>::type;
}  // namespace detail

/// A box full of voxels, where each voxel keeps track of its own boundary
/// and indices of triangles that overlap that boundary.
/// Can be 'flattened' - converts the collection into a memory-efficient
/// array representation, which can be passed to the GPU.
template <size_t dimensions>
class voxel_collection final
        : public detail::voxel_collection_data<dimensions> {
public:
    using data_type = detail::voxel_collection_data_t<dimensions>;

    /// Construct directly from an existing tree.
    voxel_collection(const ndim_tree<dimensions>& tree)
            : detail::voxel_collection_data<dimensions>(tree)
            , aabb(tree.get_aabb()) {}

    box<dimensions> get_aabb() const { return aabb; }

    size_t get_side() const { return this->get_data().size(); }

    auto get_starting_index(const glm::vec3& position) const {
        return glm::floor((position - get_aabb().get_c0()) /
                          ::dimensions(this->get_voxel_aabb()));
    }

    /// Returns a flat array-representation of the collection.
    /// TODO document the array format
    aligned::vector<cl_uint> get_flattened() const;

    using traversal_callback = std::function<geo::intersection(
            const geo::ray&, const aligned::vector<size_t>&)>;

    /// Find the closest object along a ray.
    geo::intersection traverse(const geo::ray& ray,
                               const traversal_callback& fun) const {
        auto ind = get_starting_index(ray.get_position());
        const auto voxel_bounds = this->get_voxel(ind).get_aabb();

        const auto gt = nonnegative(ray.get_direction());
        const auto step = select(glm::ivec3(1), glm::ivec3(-1), gt);
        const auto just_out =
                select(glm::ivec3(this->get_data().size()), glm::ivec3(-1), gt);
        const auto boundary =
                select(voxel_bounds.get_c1(), voxel_bounds.get_c0(), gt);

        const auto t_max_temp =
                glm::abs((boundary - ray.get_position()) / ray.get_direction());
        auto t_max = select(t_max_temp,
                            glm::vec3(std::numeric_limits<float>::infinity()),
                            is_not_nan(t_max_temp));
        const auto t_delta =
                glm::abs(::dimensions(voxel_bounds) / ray.get_direction());

        for (;;) {
            const auto min_i = min_component(t_max);

            const auto& tri = this->get_voxel(ind).get_items();
            if (!tri.empty()) {
                const auto ret = fun(ray, tri);

                if (ret && ret->distance < t_max[min_i]) {
                    return ret;
                }
            }

            ind[min_i] += step[min_i];
            if (ind[min_i] == just_out[min_i]) {
                return geo::intersection();
            }
            t_max[min_i] += t_delta[min_i];
        }
    }

private:
    static glm::bvec3 nonnegative(const glm::vec3& t) {
        glm::bvec3 ret;
        for (auto i = 0u; i != t.length(); ++i) {
            ret[i] = 0 <= t[i];
        }
        return ret;
    }

    template <typename T, typename U>
    static T select(const T& a, const T& b, const U& selector) {
        T ret;
        for (auto i = 0u; i != selector.length(); ++i) {
            ret[i] = selector[i] ? a[i] : b[i];
        }
        return ret;
    }

    static auto min_component(const glm::vec3& v) {
        size_t ret{0};
        for (auto i = 1u; i != 3; ++i) {
            if (v[i] < v[ret]) {
                ret = i;
            }
        }
        return ret;
    }

    static glm::bvec3 is_not_nan(const glm::vec3& v) {
        glm::bvec3 ret;
        for (auto i = 0u; i != v.length(); ++i) {
            ret[i] = !std::isnan(v[i]);
        }
        return ret;
    }

    box<dimensions> aabb;
};
