#include "MainComponent.hpp"

#include "combined_config_serialize.h"
#include "common/surface_serialize.h"

File check_exists(const File& root, const std::string& file) {
    auto f = root.getChildFile(file.c_str());
    if (!f.existsAsFile()) {
        throw std::runtime_error("project must contain " +
                                 f.getRelativePathFrom(root).toStdString());
    }
    return f;
}

SceneData load_model(const File& root) {
    auto f = check_exists(root, "model.model");
    return SceneData(f.getFullPathName().toStdString());
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

MainContentComponent::MainContentComponent(const File& root)
        : model(load_model(root))
        , materials(load_materials(root))
        , config(load_config(root))
        , modelRendererComponent(model, config) {
    model.set_surfaces(materials);
    addAndMakeVisible(modelRendererComponent);
    setSize(600, 400);
}

void MainContentComponent::paint(Graphics& g) {
}

void MainContentComponent::resized() {
    modelRendererComponent.setBounds(getLocalBounds());
}

void MainContentComponent::save_as_project() {
    FileChooser fc("save project", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        //  TODO
    }
}