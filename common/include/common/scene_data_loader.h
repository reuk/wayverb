#pragma once

#include <memory>

class scene_data;

class scene_data_loader final {
public:
    scene_data_loader() = default;
    scene_data_loader(const std::string& fpath);

    //  need to declare but not define these here because pimpl idiom wew
    scene_data_loader(scene_data_loader&&) noexcept;
    scene_data_loader& operator=(scene_data_loader&&) noexcept;
    ~scene_data_loader() noexcept;

    void load(const std::string& f);
    void save(const std::string& f) const;

    bool is_loaded() const;
    void clear();

    const scene_data& get_scene_data() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};
