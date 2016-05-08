#include "RightPanel.hpp"

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

RightPanel::RightPanel(RenderStateManager& render_state_manager, const File& root)
        : render_state_manager(render_state_manager)
        , model(load_model(root, load_materials(root)))
        , config(load_config(root))
        , model_renderer_component(render_state_manager, model, config) {
    addAndMakeVisible(model_renderer_component);
}

void RightPanel::resized() {
/*
    auto panel_height = 30;
    model_renderer_component.setBounds(
        getLocalBounds().withTrimmedBottom(panel_height));
    bottom_panel.setBounds(getLocalBounds().removeFromBottom(panel_height));
*/
    model_renderer_component.setBounds(getLocalBounds());
}

void RightPanel::save_as_project() {
    FileChooser fc("save project", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        //  TODO
    }
}
