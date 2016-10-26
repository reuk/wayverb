#pragma once

#include "raytracer/cl/structs.h"

#include "common/nan_checking.h"

#include "utilities/aligned/vector.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace raytracer {

template <typename Impulse, typename T>
void impulse_check(const Impulse& i, T t) {
    throw_if_suspicious(i.volume);
    throw_if_suspicious(i.position);
    throw_if_suspicious(i.distance);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Impulse>
class results final {
public:
    template <typename A, typename B>
    results(A b_image_source,
            A e_image_source,
            B b_diffuse,
            B e_diffuse,
            const glm::vec3& receiver)
            : data_{[&] {
                aligned::vector<Impulse> ret;
                ret.reserve(std::distance(b_image_source, e_image_source) +
                            std::distance(b_diffuse, e_diffuse));
                ret.insert(ret.end(), b_image_source, e_image_source);
                ret.insert(ret.end(), b_diffuse, e_diffuse);
                return ret;
            }()}
            , separator_{std::distance(b_image_source, e_image_source)}
            , receiver_{receiver} {
        for (const auto& i : data_) {
            impulse_check(i, [](auto i) {
                using std::isnan;
                return isnan(i);
            });
            impulse_check(i, [](auto i) {
                using std::isinf;
                return isinf(i);
            });
        }
    }

    auto image_source_begin() const { return data_.begin(); }
    auto image_source_begin() { return data_.begin(); }

    auto image_source_end() const { return data_.begin() + separator_; }
    auto image_source_end() { return data_.begin() + separator_; }

    auto diffuse_begin() const { return data_.begin() + separator_; }
    auto diffuse_begin() { return data_.begin() + separator_; }

    auto diffuse_end() const { return data_.end(); }
    auto diffuse_end() { return data_.end(); }

    auto begin() const { return data_.begin(); }
    auto begin() { return data_.begin(); }

    auto end() const { return data_.end(); }
    auto end() { return data_.end(); }

    const auto& get_data() const { return data_; }
    auto& get_data() { return data_; }

private:
    aligned::vector<Impulse> data_;
    ptrdiff_t separator_;
    glm::vec3 receiver_;
};

}  // namespace raytracer
