#include "raytracer/image_source_tree.h"

#include "common/aligned/set.h"
#include "common/voxel_collection.h"

//  I want to be able to
//      add paths one-by-one
//      build a tree of all the unique paths
//      have some way of iterating unique paths recursively?

namespace {

template <typename T, typename compare = std::less<T>>
class node final {
public:
    template <typename... Ts>
    node(const T& t, Ts&&... ts)
            : t(t)
            , pimpl(std::make_unique<impl>(std::forward<Ts>(ts)...)) {}
    template <typename... Ts>
    node(T&& t, Ts&&... ts)
            : t(std::move(t))
            , pimpl(std::make_unique<impl>(std::forward<Ts>(ts)...)) {}

    node(const node& rhs)
            : pimpl(std::make_unique<impl>(*rhs.pimpl)) {}

    node& operator=(node rhs) {
        swap(rhs);
        return *this;
    }

    ~node() noexcept;

    const node* get_up() const { return pimpl->get_up(); }

    const T& value() const { return t; }

    void clear() const { pimpl->down.clear(); }
    void erase(const T& t) const { pimpl->down.erase(make_node(t)); }

    auto insert(T&& t) const {
        return pimpl->down.insert(make_node(std::move(t)));
    }
    auto insert(const T& t) const { return pimpl->down.insert(make_node(t)); }

    auto find(const T& t) const { return pimpl->down.find(make_node(t)); }

    void swap(node& rhs) noexcept {
        using std::swap;
        swap(std::tie(pimpl, t), std::tie(rhs.pimpl, rhs.t));
    }

    template <typename Func>
    void walk(const Func& func) const {
        auto next = func(value());
        for (const auto& i : pimpl->down) {
            i.walk(next);
        }
    }

private:
    auto make_node(T&& t) const {
        return node(std::move(t), pimpl->down.value_comp().get_comp());
    }

    auto make_node(const T& t) const {
        return node(t, pimpl->down.value_comp().get_comp());
    }

    T t;
    class impl;
    const std::unique_ptr<impl> pimpl;
};

template <typename T, typename compare>
class node<T, compare>::impl final {
    class comparator final {
    public:
        comparator()
                : comparator(compare()) {}
        explicit comparator(const compare& comp)
                : comp(comp) {}
        constexpr bool operator()(const node& a, const node& b) const {
            return comp(a.value(), b.value());
        }

        compare get_comp() const { return comp; }

    private:
        compare comp;
    };

public:
    impl()
            : impl(compare()) {}
    impl(const compare& comp)
            : down(comparator(comp)) {}

    aligned::set<node, comparator> down;
};

template <typename T, typename compare>
node<T, compare>::~node() noexcept = default;

template <typename T>
auto make_node(const T& t) {
    return node<T>(t);
}

template <typename T>
auto make_node(T&& t) {
    return node<T>(std::move(t));
}

}  // namespace

class image_source_tree::impl final {
public:
    void add_path(const aligned::vector<item>& path) {
        auto node = paths.insert(make_node(path.front())).first;
        for (auto it = path.begin() + 1; it != path.end(); ++it) {
            node = node->insert(*it).first;
        }
    }

private:
    template <typename Func>
    void walk(const Func& func) {
        for (const auto& i : paths) {
            i.walk(func);
        }
    }

    //  each step has
    //      original in-scene triangle
    //      mirrored image-source triangle
    //      mirrored receiver position
    //
    //  for each step
    //      if the receiver is visible
    //          cast a ray through the current path
    //          find the distances to the intersections
    //          mirror them back into scene space
    //          attempt to trace the same path in scene space
    //          if the path is valid

    class path_walker final {
    public:
        path_walker(const glm::vec3& source,
                    const glm::vec3& receiver,
                    const CopyableSceneData& scene_data,
                    const VoxelCollection& vox,
                    const VoxelCollection::TriangleTraversalCallback& callback,
                    aligned::vector<Impulse>& ret)
                : source(source)
                , receiver(receiver)
                , scene_data(scene_data)
                , vox(vox)
                , callback(callback)
                , ret(ret) {}

        path_walker(const glm::vec3& source,
                    const glm::vec3& receiver,
                    const CopyableSceneData& scene_data,
                    const VoxelCollection& vox,
                    const VoxelCollection::TriangleTraversalCallback& callback,
                    aligned::vector<Impulse>& ret,
                    const item& item)
                : source(source)
                , receiver(receiver)
                , scene_data(scene_data)
                , vox(vox)
                , callback(callback)
                , ret(ret) {}

        path_walker operator()(const item& item) const {
            return path_walker(
                    source, receiver, scene_data, vox, callback, ret, item);
        }

    private:
        const glm::vec3& source;
        const glm::vec3& receiver;
        const CopyableSceneData& scene_data;
        const VoxelCollection& vox;
        const VoxelCollection::TriangleTraversalCallback& callback;
        aligned::vector<Impulse>& ret;

        const aligned::vector<TriangleVec3> original_triangles;
        const aligned::vector<TriangleVec3> mirrored_triangles;
        const glm::vec3 receiver_image;
    };

    aligned::vector<Impulse> find_impulses(
            const glm::vec3& source,
            const glm::vec3& receiver,
            const CopyableSceneData& scene_data,
            const VoxelCollection& vox,
            const VoxelCollection::TriangleTraversalCallback& callback) {
        aligned::vector<Impulse> ret;
        walk(path_walker(source, receiver, scene_data, vox, callback, ret));
        return ret;
    }

    class comparator final {
    public:
        template <typename T>
        constexpr bool operator()(const T& a, const T& b) const {
            return a.value() < b.value();
        }
    };

    aligned::set<node<item>, comparator> paths;
};

image_source_tree::image_source_tree()
        : pimpl(std::make_unique<impl>()) {}
image_source_tree::~image_source_tree() noexcept                   = default;
image_source_tree::image_source_tree(image_source_tree&&) noexcept = default;
image_source_tree& image_source_tree::operator=(image_source_tree&&) noexcept =
        default;
