#include "CommandIDs.h"
#include "ImpulseViewer.hpp"
#include "Main.hpp"

ImpulseViewer::DefaultAudioFormatManager::DefaultAudioFormatManager() {
    registerBasicFormats();
}

ImpulseViewer::DefaultAudioDeviceManager::DefaultAudioDeviceManager() {
    initialiseWithDefaultDevices(0, 2);
}

ImpulseViewer::ImpulseViewer(const File& file)
        : audio_format_reader_source(audio_format_manager.createReaderFor(file),
                                     true)
        , renderer(audio_transport_source)
        , tabs(TabbedButtonBar::Orientation::TabsAtTop)
        , transport(audio_transport_source) {
    audio_transport_source.setSource(&audio_format_reader_source);
    audio_source_player.setSource(&audio_transport_source);
    audio_device_manager.addAudioCallback(&audio_source_player);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    //    addKeyListener(command_manager.getKeyMappings());

    tabs.addTab("waveform", Colours::darkgrey, -1);
    tabs.addTab("waterfall", Colours::darkgrey, -1);

    addAndMakeVisible(renderer);
    addAndMakeVisible(tabs);
    addAndMakeVisible(transport);

    setSize(800, 500);

    load_from(file);
}

ImpulseViewer::~ImpulseViewer() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void ImpulseViewer::resized() {
    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(40);
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));
    tabs.setBounds(top);
}

void ImpulseViewer::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &tabs) {
        switch (tabs.getCurrentTabIndex()) {
            case 0:
                renderer.get_renderer().set_mode(
                        ImpulseRenderer::Mode::waveform);
                break;
            case 1:
                renderer.get_renderer().set_mode(
                        ImpulseRenderer::Mode::waterfall);
                break;
        }
    }
}

void ImpulseViewer::load_from(const File& file) {
    renderer.get_renderer().load_from(audio_format_manager, file);
}

void ImpulseViewer::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idPlay,
                       CommandIDs::idPause,
                       CommandIDs::idReturnToBeginning});
}
void ImpulseViewer::getCommandInfo(CommandID command_id,
                                   ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idPlay:
            result.setInfo("Play", "Start playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(!audio_transport_source.isPlaying());
            break;

        case CommandIDs::idPause:
            result.setInfo("Pause", "Pause playback", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress(KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
            result.setActive(audio_transport_source.isPlaying());
            break;

        case CommandIDs::idReturnToBeginning:
            result.setInfo("Go to Start",
                           "Move playhead to the beginning",
                           "General",
                           0);
            result.defaultKeypresses.add(KeyPress(
                    KeyPress::returnKey, ModifierKeys::noModifiers, 0));
            break;
        default:
            break;
    }
}
bool ImpulseViewer::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idPlay:
            audio_transport_source.start();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idPause:
            audio_transport_source.stop();
            VisualiserApplication::get_command_manager().commandStatusChanged();
            return true;

        case CommandIDs::idReturnToBeginning:
            audio_transport_source.setPosition(0);
            return true;

        default:
            return false;
    }
}
ApplicationCommandTarget* ImpulseViewer::getNextCommandTarget() {
    return findFirstTargetParentComponent();
}