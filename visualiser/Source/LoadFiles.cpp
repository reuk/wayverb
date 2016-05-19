#include "LoadFiles.hpp"

#include "combined/config_serialize.h"
#include "common/surface_serialize.h"

#include <fstream>

File check_exists(const File& root, const std::string& file) {
    auto f = root.getChildFile(file.c_str());
    if (!f.existsAsFile()) {
        throw std::runtime_error("project must contain " +
                                 f.getRelativePathFrom(root).toStdString());
    }
    return f;
}

SceneData load_model(const File& root, const SurfaceConfig& sc) {
    auto f = check_exists(root, "model.model");
    SceneData ret(f.getFullPathName().toStdString());
    ret.set_surfaces(sc);

    //    auto v = 1.0f;
    //    ret.set_surfaces(
    //        Surface{{{v, v, v, v, v, v, v, v}}, {{v, v, v, v, v, v, v, v}}});

    return ret;
}

SurfaceConfig load_materials(const File& root) {
    auto f = check_exists(root, "materials.json");
    std::ifstream stream(f.getFullPathName().toStdString());
    cereal::JSONInputArchive archive(stream);
    SurfaceConfig materials;
    materials.serialize(archive);
    return materials;
}

config::Combined load_config(const File& root) {
    auto f = check_exists(root, "config.json");
    std::ifstream stream(f.getFullPathName().toStdString());
    cereal::JSONInputArchive archive(stream);
    config::Combined config;
    archive(cereal::make_nvp("config", config));

    config.rays = 64;

    return config;
}

void save_materials(const File& root, const SurfaceConfig& materials) {
}

void save_config(const File& root, const config::Combined& config) {
}
