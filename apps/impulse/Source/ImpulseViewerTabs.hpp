#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "UtilityComponents/DefaultAudio.hpp"

class ImpulseViewerTabs : public TabbedComponent,
                          public ApplicationCommandTarget {
public:
    ImpulseViewerTabs(const File& impulse_file);

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void show_audio_preferences();

private:
    DefaultAudioDeviceManager audio_device_manager;
    DefaultAudioFormatManager audio_format_manager;
};