#pragma once

#include "combined/model/constrained_point.h"
#include "combined/model/hover.h"
#include "combined/model/vector.h"

#include "core/geo/box.h"

#include "cereal/types/base_class.hpp"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class source final
        : public owning_member<source, constrained_point, hover_state> {
public:
    explicit source(const core::geo::box& aabb);

    void set_name(std::string name);
    std::string get_name() const;

    auto& position() { return get<constrained_point>(); }
    const auto& position() const { return get<constrained_point>(); }

    using hover_state_t = class hover_state;
    auto& hover_state() { return get<hover_state_t>(); }
    const auto& hover_state() const { return get<hover_state_t>(); }

    template <class Archive>
    static void load_and_construct(Archive& ar,
                                   cereal::construct<source>& construct) {
        core::geo::box aabb;
        ar(aabb);
        construct(aabb);
    }

    template <typename Archive>
    void load(Archive& archive) {
        archive(position(), name_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(position(), name_);
    }

private:
    std::string name_;
};

////////////////////////////////////////////////////////////////////////////////

class sources final : public owning_member<sources, vector<source, 1>> {
public:
    explicit sources(const core::geo::box& aabb);

    const shared_value<source>& operator[](size_t index) const;
    shared_value<source>& operator[](size_t index);

    auto cbegin() const { return data()->cbegin(); }
    auto begin() const { return data()->begin(); }
    auto begin() { return data()->begin(); }

    auto cend() const { return data()->cend(); }
    auto end() const { return data()->end(); }
    auto end() { return data()->end(); }

    template <typename It>
    void insert(It it) {
        data()->insert(std::move(it), source{aabb_});
    }

    template <typename It>
    void erase(It it) {
        data()->erase(std::move(it));
    }

    size_t size() const;
    bool empty() const;

    void clear();

    bool can_erase() const;

    template <class Archive>
    static void load_and_construct(Archive& ar,
                                   cereal::construct<sources>& construct) {
        core::geo::box aabb;
        ar(aabb);
        construct(aabb);
    }

private:
    const shared_value<vector<source, 1>>& data() const;
    shared_value<vector<source, 1>>& data();

    core::geo::box aabb_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
