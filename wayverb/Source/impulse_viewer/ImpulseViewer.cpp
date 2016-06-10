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
        , waterfall_button("waterfall")
        , waveform_button("waveform")
        , transport(audio_transport_source) {
    audio_transport_source.setSource(&audio_format_reader_source);
    audio_source_player.setSource(&audio_transport_source);
    audio_device_manager.addAudioCallback(&audio_source_player);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);

    auto r =
            Range<float>(0, audio_transport_source.getLengthInSeconds());
    ruler.set_max_range(r);
    ruler.set_visible_range(r, true);

    for (auto i : {&waterfall_button, &waveform_button}) {
        i->setRadioGroupId(0xf);
        i->setClickingTogglesState(true);
        i->setWantsKeyboardFocus(false);
    }

    waveform_button.setToggleState(true, sendNotification);

    addAndMakeVisible(renderer);
    addAndMakeVisible(waterfall_button);
    addAndMakeVisible(waveform_button);
    addAndMakeVisible(transport);
    addAndMakeVisible(ruler);
    setSize(800, 500);

    load_from(file);
}

ImpulseViewer::~ImpulseViewer() noexcept {
    audio_device_manager.removeAudioCallback(&audio_source_player);
}

void ImpulseViewer::resized() {
    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(40);
    top.reduce(2, 2);
    ruler.setBounds(bounds.removeFromTop(20));
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));

    waveform_button.setBounds(top.removeFromLeft(100));
    top.removeFromLeft(2);
    waterfall_button.setBounds(top.removeFromLeft(100));
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

void ImpulseViewer::buttonClicked(Button* b) {
    if (b == &waterfall_button) {
        renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waterfall);
    } else if (b == &waveform_button) {
        renderer.get_renderer().set_mode(ImpulseRenderer::Mode::waveform);
    }
}

/*
void ImpulseViewer::rulerDragged(Ruler* ruler, const MouseEvent& e) {
    assert(ruler_state);

    auto dy = e.getDistanceFromDragStartY();

    auto doubleDist = 100.0;
    auto scale = pow(2.0, dy / doubleDist);

    auto w = static_cast<int>(startWidth * scale);
    auto clamped = std::max(getParentWidth(), w);

    if (audio_transport_source.getTotalLength() - 1 > getParentWidth()) {
        clamped =
                std::min(clamped, audio_transport_source.getTotalLength() - 1);
    }

    setSize(clamped, getHeight());

    auto dx = e.getDistanceFromDragStartX();
    auto timePos = mouseDownTime * getWidth() /
                   audioTransportSource.getLengthInSeconds();
    setTopLeftPosition(dx + mouseDownX - timePos, 0);
}
*/