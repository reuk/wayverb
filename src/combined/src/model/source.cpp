#include "combined/model/source.h"

namespace wayverb {
namespace combined {
namespace model {

source::source(core::geo::box aabb)
        : aabb_{std::move(aabb)}
        , name_{"new source"}
        , position_{centre(aabb_)} {}

void source::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string source::get_name() const { return name_; }

void source::set_position(const glm::vec3& position) {
    position_ = clamp(position, aabb_);
    notify();
}

glm::vec3 source::get_position() const { return position_; }

////////////////////////////////////////////////////////////////////////////////

sources::sources(core::geo::box aabb)
        : aabb_{std::move(aabb)} {
    insert(0, source{aabb_});
    connect(sources_);
}

void sources::swap(sources& other) noexcept {
    using std::swap;
    swap(aabb_, other.aabb_);
    swap(sources_, other.sources_);
}

sources::sources(const sources& other)
        : aabb_{other.aabb_}
        , sources_{other.sources_} {
    connect(sources_);
}

sources::sources(sources&& other) noexcept {
    swap(other);
    connect(sources_);
}

sources& sources::operator=(const sources& other) {
    auto copy{other};
    swap(copy);
    connect(sources_);
    return *this;
}

sources& sources::operator=(sources&& other) noexcept {
    swap(other);
    connect(sources_);
    return *this;
}

const source& sources::operator[](size_t index) const {
    return sources_[index];
}
source& sources::operator[](size_t index) { return sources_[index]; }

void sources::insert(size_t index, source t) {
    sources_.insert(sources_.begin() + index, std::move(t));
}

void sources::erase(size_t index) { 
    if (1 < sources_.size()) {
        sources_.erase(sources_.begin() + index);
    }
}

size_t sources::size() const { return sources_.size(); }

bool sources::empty() const { return sources_.empty(); }

void sources::clear() { sources_.clear(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
