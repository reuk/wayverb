#pragma once

#include "DefaultAudio.hpp"

class ImpulseViewerTabs : public TabbedComponent {
public:
    ImpulseViewerTabs(const File& impulse_file);
private:
    DefaultAudioDeviceManager audio_device_manager;
    DefaultAudioFormatManager audio_format_manager;
};