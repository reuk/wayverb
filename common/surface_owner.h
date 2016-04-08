#pragma once

#include "cl_include.h"

using VolumeType = cl_float8;
std::ostream& operator<<(std::ostream& os, const VolumeType& f);

typedef struct {
    VolumeType specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    VolumeType diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
} __attribute__((aligned(8))) Surface;

std::ostream& operator<<(std::ostream& os, const Surface& s);

constexpr bool validate_surface(const Surface& s,
                                float min_gain = 0.001,
                                float max_gain = 0.999) {
    bool ret = true;
    for (auto i = 0; i != sizeof(VolumeType) / sizeof(cl_float); ++i) {
        ret = ret && min_gain <= s.specular.s[i] &&
              s.specular.s[i] <= max_gain && min_gain <= s.diffuse.s[i] &&
              s.diffuse.s[i] <= max_gain;
    }
    return ret;
}

class SurfaceLoader;

class SurfaceOwner {
public:
    SurfaceOwner(const std::vector<Surface>& surfaces);
    SurfaceOwner(std::vector<Surface>&& surfaces);
    SurfaceOwner(const SurfaceLoader& surface_loader);
    SurfaceOwner(SurfaceLoader&& surface_loader);

    const std::vector<Surface>& get_surfaces() const;

private:
    void check_num_surfaces() const;
    std::vector<Surface> surfaces;
};
