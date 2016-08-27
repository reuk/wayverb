#include "raytracer/image_source_tree.h"

namespace raytracer {

multitree<path_element>::branches construct_image_source_tree(
        const aligned::vector<aligned::vector<path_element>>& paths) {
    multitree<path_element> root{path_element{}};
    for (const auto& i : paths) {
        root.add_path(i.begin(), i.end());
    }
    return std::move(root.branches_);
}

}  // namespace raytracer
