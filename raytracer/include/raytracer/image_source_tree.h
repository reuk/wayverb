#pragma once

#include "common/aligned/vector.h"
#include "raytracer/cl_structs.h"

#include <memory>

class image_source_tree final {
public:
    image_source_tree();
    ~image_source_tree() noexcept;

    image_source_tree(image_source_tree&&) noexcept;
    image_source_tree& operator=(image_source_tree&&) noexcept;

    struct item final {
        cl_ulong index;
        bool visible;

        constexpr bool operator<(const item& a) const {
            return index < a.index;
        }
    };

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};
