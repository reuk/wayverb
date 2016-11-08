#pragma once

#include "combined/model/vector.h"

#include "core/geo/box.h"

#include <string>

namespace wayverb {
namespace combined {
namespace model {

class source final : public member<source> {
public:
    source(core::geo::box aabb);

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const;

private:
    core::geo::box aabb_;

    std::string name_;
    glm::vec3 position_;
};

////////////////////////////////////////////////////////////////////////////////

class sources final : public member<sources, vector<source>> {
public:
    sources(core::geo::box aabb);

    sources(const sources& other);
    sources(sources&& other) noexcept;

    sources& operator=(const sources& other);
    sources& operator=(sources&& other) noexcept;

    void swap(sources& other) noexcept;

    const source& operator[](size_t index) const;
    source& operator[](size_t index);

    auto cbegin() const { return sources_.cbegin(); }
    auto begin() const { return sources_.begin(); }
    auto begin() { return sources_.begin(); }

    auto cend() const { return sources_.cend(); }
    auto end() const { return sources_.end(); }
    auto end() { return sources_.end(); }

    void insert(size_t index, source t);
    void erase(size_t index);

    size_t size() const;
    bool empty() const;

    void clear();

private:
    core::geo::box aabb_;
    vector<source> sources_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
