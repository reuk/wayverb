#pragma once

#include "combined_config.h"
#include "common/scene_data.h"

#include "../JuceLibraryCode/JuceHeader.h"

File check_exists(const File& root, const std::string& file);
SceneData load_model(const File& root, const SurfaceConfig& sc);
SurfaceConfig load_materials(const File& root);
config::Combined load_config(const File& root);

void save_materials(const File& root, const SurfaceConfig& materials);
void save_config(const File& root, const config::Combined& config);
