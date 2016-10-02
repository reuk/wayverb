#pragma once

#include "raytracer/cl/structs.h"

#include "common/aligned/vector.h"
#include "common/nan_checking.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace raytracer {

template <typename impulse, typename T>
void impulse_check(const impulse& i, T t) {
    throw_if_suspicious(i.volume);
    throw_if_suspicious(i.position);
    throw_if_suspicious(i.distance);
}

template <typename impulse>
class results final {
public:
    results(std::experimental::optional<impulse> direct,
            aligned::vector<impulse> image_source,
            aligned::vector<aligned::vector<impulse>> diffuse,
            const glm::vec3& receiver)
            : direct_{std::move(direct)}
            , image_source_{std::move(image_source)}
            , diffuse_{std::move(diffuse)}
            , receiver_{receiver} {
        //  Do a quick test to make sure the results look alright
        for_each_impulse([](const auto& i) {
            impulse_check(i, [](auto i) {
                using std::isnan;
                return isnan(i);
            });
            impulse_check(i, [](auto i) {
                using std::isinf;
                return isinf(i);
            });
        });
    }

    aligned::vector<impulse> get_impulses(bool direct = true,
                                          bool image_source = true,
                                          bool diffuse = true) const {
        const size_t direct_size = direct ? 1 : 0;
        const size_t image_source_size =
                image_source ? image_source_.size() : 0;
        const size_t diffuse_size =
                diffuse && diffuse_.size()
                        ? diffuse_.size() * diffuse_.front().size()
                        : 0;

        aligned::vector<impulse> ret;
        ret.reserve(direct_size + image_source_size + diffuse_size);

        for_each_impulse([&](const auto& i) { ret.emplace_back(i); },
                         direct,
                         image_source,
                         diffuse);

        return ret;
    }

    using impulse_callback = std::function<void(const impulse&)>;
    void for_each_impulse(const impulse_callback& callback,
                          bool direct = true,
                          bool image_source = true,
                          bool diffuse = true) const {
        if (!callback) {
            throw std::runtime_error(
                    "no callback passed to results::for_each_impulse!");
        }

        if (direct && direct_) {
            callback(*direct_);
        }

        if (image_source) {
            for (const auto& i : image_source_) {
                callback(i);
            }
        }

        if (diffuse) {
            for (const auto& i : diffuse_) {
                for (const auto& j : i) {
                    callback(j);
                }
            }
        }
    }

    const std::experimental::optional<impulse>& get_direct() const {
        return direct_;
    }
    const aligned::vector<impulse>& get_image_source() const {
        return image_source_;
    }
    const aligned::vector<aligned::vector<impulse>>& get_diffuse() const {
        return diffuse_;
    }

    glm::vec3 get_receiver() const { return receiver_; }

private:
    std::experimental::optional<impulse> direct_;
    aligned::vector<impulse> image_source_;
    aligned::vector<aligned::vector<impulse>> diffuse_;

    glm::vec3 receiver_;
};

}  // namespace raytracer
