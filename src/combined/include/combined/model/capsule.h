#pragma once

#include "combined/model/hrtf.h"
#include "combined/model/microphone.h"
#include "combined/model/vector.h"

namespace wayverb {
namespace combined {
namespace model {

class capsule final : public member<capsule, microphone, hrtf> {
public:
    capsule();

    capsule(const capsule& other);
    capsule(capsule&& other) noexcept;

    capsule& operator=(const capsule& other);
    capsule& operator=(capsule&& other) noexcept;

    void swap(capsule& other) noexcept;

    enum class mode { microphone, hrtf };

    void set_name(std::string name);
    std::string get_name() const;

    void set_mode(mode mode);
    mode get_mode() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(microphone, hrtf, name_, mode_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(microphone, hrtf, name_, mode_);
    }

    microphone microphone;
    hrtf hrtf;

private:
    std::string name_ = "new capsule";
    mode mode_ = mode::microphone;
};

////////////////////////////////////////////////////////////////////////////////

class capsules final : public member<capsules, vector<capsule>> {
public:
    capsules();

    capsules(const capsules& other);
    capsules(capsules&& other) noexcept;

    capsules& operator=(const capsules& other);
    capsules& operator=(capsules&& other) noexcept;

    void swap(capsules& other) noexcept;

    const capsule& operator[](size_t index) const;
    capsule& operator[](size_t index);

    auto cbegin() const { return capsules_.cbegin(); }
    auto begin() const { return capsules_.begin(); }
    auto begin() { return capsules_.begin(); }

    auto cend() const { return capsules_.cend(); }
    auto end() const { return capsules_.end(); }
    auto end() { return capsules_.end(); }

    void insert(size_t index, capsule t);
    void erase(size_t index);

    size_t size() const;
    bool empty() const;

    void clear();

    template <typename Archive>
    void load(Archive& archive) {
        archive(capsules_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(capsules_);
    }

private:
    vector<capsule> capsules_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
