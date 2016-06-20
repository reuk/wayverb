#pragma once

#include "FileDropComponent.hpp"

class ConvolutionLoader : public Component, public FileDropComponent::Listener {
public:
    ConvolutionLoader(AudioDeviceManager& audio_device_manager,
                      AudioFormatManager& audio_format_manager);

    void resized() override;

    void file_dropped(FileDropComponent* f, const File& file) override;

    void set_file_loader();
    void set_convolver(const File& f);

private:
    void set_content(std::unique_ptr<Component>&& c);

    AudioDeviceManager& audio_device_manager;
    AudioFormatManager& audio_format_manager;

    std::unique_ptr<Component> content_component;
};
