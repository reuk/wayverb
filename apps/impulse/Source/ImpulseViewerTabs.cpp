#include "ImpulseViewerTabs.hpp"

#include "Application.hpp"
#include "CommandIDs.hpp"
#include "ImpulseViewer.hpp"

#include "ConvolutionComponent/ConvolutionLoader.hpp"

ImpulseViewerTabs::ImpulseViewerTabs(const File& impulse_file)
        : TabbedComponent(TabbedButtonBar::TabsAtTop) {
    auto& command_manager = ImpulseApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);

    addTab("ir inspector",
           Colours::darkgrey,
           new ImpulseViewer(
                   audio_device_manager, audio_format_manager, impulse_file),
           true);

    addTab("auralisation",
           Colours::darkgrey,
           new ConvolutionLoader(audio_device_manager, audio_format_manager),
           true);

    setWantsKeyboardFocus(false);
    //    getTabContentComponent(0)->grabKeyboardFocus();
}

void ImpulseViewerTabs::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idShowAudioPreferences});
}

void ImpulseViewerTabs::getCommandInfo(CommandID command_id,
                                       ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idShowAudioPreferences:
            result.setInfo("Show Audio Preferences",
                           "Open a window to modify audio settings",
                           "General",
                           0);
            break;

        default:
            break;
    }
}

bool ImpulseViewerTabs::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idShowAudioPreferences:
            show_audio_preferences();
            return true;

        default:
            return false;
    }
}

ApplicationCommandTarget* ImpulseViewerTabs::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}

void ImpulseViewerTabs::show_audio_preferences() {
    DialogWindow::LaunchOptions launchOptions;
    launchOptions.dialogTitle = "preferences";
    launchOptions.dialogBackgroundColour = Colours::darkgrey;
    launchOptions.escapeKeyTriggersCloseButton = true;
    launchOptions.useNativeTitleBar = true;
    launchOptions.resizable = false;
    launchOptions.useBottomRightCornerResizer = false;

    auto content = std::make_unique<AudioDeviceSelectorComponent>(
            audio_device_manager, 0, 0, 0, 2, false, false, false, false);
    content->setSize(400, 400);
    launchOptions.content.setOwned(content.release());
    launchOptions.launchAsync();
}
