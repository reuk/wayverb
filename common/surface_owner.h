#pragma once

#include "cl_include.h"
#include "json_read_write.h"

#include <cereal/types/vector.hpp>

using VolumeType = cl_float8;

template <typename Archive>
void serialize(Archive& archive, VolumeType& m) {
    archive(cereal::make_nvp("volumes", m.s));
}

JSON_OSTREAM_OVERLOAD(VolumeType)

typedef struct {
    VolumeType specular{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
    VolumeType diffuse{{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}};
} __attribute__((aligned(8))) Surface;

template <typename Archive>
void serialize(Archive& archive, Surface& m) {
    archive(cereal::make_nvp("specular", m.specular),
            cereal::make_nvp("diffuse", m.diffuse));
}

JSON_OSTREAM_OVERLOAD(Surface)

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
    explicit SurfaceOwner(const std::vector<Surface>& surfaces);
    explicit SurfaceOwner(std::vector<Surface>&& surfaces);
    explicit SurfaceOwner(const SurfaceLoader& surface_loader);
    explicit SurfaceOwner(SurfaceLoader&& surface_loader);

    const std::vector<Surface>& get_surfaces() const;

    auto& surface_at(size_t index) {
        return surfaces.at(index);
    }
    const auto& surface_at(size_t index) const {
        return surfaces.at(index);
    }

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(surfaces));
    }

private:
    void check_num_surfaces() const;
    std::vector<Surface> surfaces;
};

JSON_OSTREAM_OVERLOAD(SurfaceOwner)
