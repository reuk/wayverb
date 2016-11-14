#pragma once

#include "combined/model/constrained_point.h"
#include "combined/model/vector.h"

#include "core/geo/box.h"

#include "cereal/types/base_class.hpp"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class source final : public owning_member<source, constrained_point> {
    friend class vector<source, 1>;
    source() = default;

public:
    explicit source(const core::geo::box& aabb);

    void set_name(std::string name);
    std::string get_name() const;

    auto& position() { return get<constrained_point>(); }
    const auto& position() const { return get<constrained_point>(); }

    template <typename Archive>
    void load(Archive& archive) {
        archive(cereal::base_class<type>(this), name_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(cereal::base_class<type>(this), name_);
    }

    struct raw final {
        std::string name;
        glm::vec3 position;
    };

    raw get_raw() const;

private:
    std::string name_;
};

////////////////////////////////////////////////////////////////////////////////

class sources final : public owning_member<sources, vector<source, 1>> {
public:
    explicit sources(const core::geo::box& aabb);

    const source& operator[](size_t index) const;
    source& operator[](size_t index);

    auto cbegin() const { return data().cbegin(); }
    auto begin() const { return data().begin(); }
    auto begin() { return data().begin(); }

    auto cend() const { return data().cend(); }
    auto end() const { return data().end(); }
    auto end() { return data().end(); }

    template <typename It>
    void insert(It it) {
        data().insert(std::move(it), source{aabb_});
    }

    template <typename It>
    void erase(It it) {
        data().erase(std::move(it));
    }

    size_t size() const;
    bool empty() const;

    void clear();

    bool can_erase() const;

    std::vector<source::raw> get_raw() const;

private:
    vector<source, 1>& data();
    const vector<source, 1>& data() const;

    core::geo::box aabb_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
