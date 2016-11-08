#pragma once

#include "combined/model/capsule.h"
#include "combined/model/vector.h"

#include "core/geo/box.h"

namespace wayverb {
namespace combined {
namespace model {

class receiver final : public member<receiver, capsules> {
public:
    receiver(core::geo::box bounds);

    receiver(const receiver& other);
    receiver(receiver&& other) noexcept;

    receiver& operator=(const receiver& other);
    receiver& operator=(receiver&& other) noexcept;

    void swap(receiver& other) noexcept;

    void set_name(std::string name);
    std::string get_name() const;

    void set_position(const glm::vec3& position);
    glm::vec3 get_position() const; 

    void set_orientation(float azimuth, float elevation);
    core::orientable get_orientation() const;

    capsules capsules;

private:
    core::geo::box bounds_;

    std::string name_ = "new receiver";
    glm::vec3 position_;
    core::orientable orientation_;
};

////////////////////////////////////////////////////////////////////////////////

class receivers final : public member<receivers, vector<receiver>> {
public:
    receivers(core::geo::box aabb);

    receivers(const receivers& other);
    receivers(receivers&& other) noexcept;

    receivers& operator=(const receivers& other);
    receivers& operator=(receivers&& other) noexcept;

    void swap(receivers& other) noexcept;

    const receiver& operator[](size_t index) const;
    receiver& operator[](size_t index);

    auto cbegin() const { return receivers_.cbegin(); }
    auto begin() const { return receivers_.begin(); }
    auto begin() { return receivers_.begin(); }

    auto cend() const { return receivers_.cend(); }
    auto end() const { return receivers_.end(); }
    auto end() { return receivers_.end(); }

    void insert(size_t index, receiver t);
    void erase(size_t index);

    size_t size() const;
    bool empty() const;

    void clear();

private:
    core::geo::box aabb_;
    vector<receiver> receivers_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
