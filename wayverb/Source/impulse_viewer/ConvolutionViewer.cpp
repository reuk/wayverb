#include "ConvolutionViewer.hpp"

#include "CommandIDs.h"
#include "ConvolutionAudioSource.hpp"
#include "ConvolutionRoutingComponent.hpp"
#include "Main.hpp"
#include "PlaybackPane.hpp"

//  As the top-level component, this should handle
//      loading the file for playback
//      transport
//      convolution (using the routing from the subcomponent)

class ConvolutionViewer : public Component,
                          public ApplicationCommandTarget,
                          public PlaybackPane::Listener {
public:
    ConvolutionViewer(AudioDeviceManager& audio_device_manager,
                      AudioFormatManager& audio_format_manager,
                      const File& f)
            : audio_device_manager(audio_device_manager)
            , audio_format_manager(audio_format_manager)
            , audio_format_reader_source(
                      audio_format_manager.createReaderFor(f), true)
            , convolution_audio_source(&audio_transport_source, false)
            , playback_pane(audio_format_manager, audio_transport_source, f)
            , resizer_bar(&layout, 1, false)
            , convolution_routing(
                      audio_device_manager,
                      audio_format_reader_source.getAudioFormatReader()
                              ->numChannels) {
        //  audio setup
        audio_transport_source.setSource(
                &audio_format_reader_source,
                0,
                nullptr,
                audio_format_reader_source.getAudioFormatReader()->sampleRate);
        playback_pane.reset_view();

        audio_source_player.setSource(&convolution_audio_source);
        audio_device_manager.addAudioCallback(&audio_source_player);

        //  command setup
        auto& command_manager = VisualiserApplication::get_command_manager();
        command_manager.registerAllCommandsForTarget(this);

        addAndMakeVisible(playback_pane);
        addAndMakeVisible(resizer_bar);
        addAndMakeVisible(viewport);

        layout.setItemLayout(0, 200, 10000, 200);
        layout.setItemLayout(1, 4, 4, 4);
        layout.setItemLayout(2, 1, 10000, 200);

        viewport.setViewedComponent(&convolution_routing, false);
    }

    virtual ~ConvolutionViewer() noexcept {
        audio_device_manager.removeAudioCallback(&audio_source_player);
    }

    void getAllCommands(Array<CommandID>& commands) override {
        commands.addArray({CommandIDs::idPlay,
                           CommandIDs::idPause,
                           CommandIDs::idReturnToBeginning});
    }

    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override {
        switch (command_id) {
            case CommandIDs::idPlay:
                result.setInfo("Play", "Start playback", "General", 0);
                result.defaultKeypresses.add(KeyPress(
                        KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
                result.setActive(!audio_transport_source.isPlaying());
                break;

            case CommandIDs::idPause:
                result.setInfo("Pause", "Pause playback", "General", 0);
                result.defaultKeypresses.add(KeyPress(
                        KeyPress::spaceKey, ModifierKeys::noModifiers, 0));
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

    bool perform(const InvocationInfo& info) override {
        switch (info.commandID) {
            case CommandIDs::idPlay:
                audio_transport_source.start();
                VisualiserApplication::get_command_manager()
                        .commandStatusChanged();
                return true;

            case CommandIDs::idPause:
                audio_transport_source.stop();
                VisualiserApplication::get_command_manager()
                        .commandStatusChanged();
                return true;

            case CommandIDs::idReturnToBeginning:
                audio_transport_source.setPosition(0);
                return true;

            default:
                return false;
        }
    }

    ApplicationCommandTarget* getNextCommandTarget() override {
        return findFirstTargetParentComponent();
    }

    void resized() override {
        std::vector<Component*> c{{&playback_pane, &resizer_bar, &viewport}};
        layout.layOutComponents(
                c.data(), c.size(), 0, 0, getWidth(), getHeight(), true, true);

        auto routing_height = std::max(convolution_routing.get_desired_height(),
                                       viewport.getHeight());
        convolution_routing.setSize(viewport.getWidth(), routing_height);
        convolution_routing.setSize(viewport.getMaximumVisibleWidth(),
                                    routing_height);
    }

    void new_file_loaded(PlaybackPane*, const File& f) override {
        listener_list.call(
                &FileDropComponent::Listener::file_dropped, nullptr, f);
    }

    void bypass_state_changed(PlaybackPane*, bool state) override {
        convolution_audio_source.set_active(state);
    }

    void addListener(FileDropComponent::Listener* l) {
        listener_list.add(l);
    }
    void removeListener(FileDropComponent::Listener* l) {
        listener_list.remove(l);
    }

private:
    AudioDeviceManager& audio_device_manager;
    AudioFormatManager& audio_format_manager;

    //  global file reader source
    AudioFormatReaderSource audio_format_reader_source;
    //  global transport
    AudioTransportSource audio_transport_source;
    //  global convolution object
    ConvolutionAudioSource convolution_audio_source;
    //  global playback object
    AudioSourcePlayer audio_source_player;

    StretchableLayoutManager layout;
    PlaybackPane playback_pane;
    StretchableLayoutResizerBar resizer_bar;
    Viewport viewport;
    ConvolutionRoutingComponent convolution_routing;

    model::Connector<PlaybackPane> playback_connector{&playback_pane, this};

    ListenerList<FileDropComponent::Listener> listener_list;
};

//----------------------------------------------------------------------------//

ConvolutionLoader::ConvolutionLoader(AudioDeviceManager& audio_device_manager,
                                     AudioFormatManager& audio_format_manager)
        : audio_device_manager(audio_device_manager)
        , audio_format_manager(audio_format_manager) {
    set_file_loader();
}

void ConvolutionLoader::set_file_loader() {
    auto c = std::make_unique<FileDropComponent>("drop an audio file here, or",
                                                 "click to load");
    c->set_valid_file_formats(
            audio_format_manager.getWildcardForAllFormats().toStdString());
    c->addListener(this);
    set_content(std::move(c));
}
void ConvolutionLoader::set_convolver(const File& f) {
    auto c = std::make_unique<ConvolutionViewer>(
            audio_device_manager, audio_format_manager, f);
    c->addListener(this);
    set_content(std::move(c));
}
void ConvolutionLoader::set_content(std::unique_ptr<Component>&& c) {
    content_component = std::move(c);
    addAndMakeVisible(*content_component);
    resized();
}

void ConvolutionLoader::resized() {
    if (content_component) {
        content_component->setBounds(getLocalBounds());
    }
}

void ConvolutionLoader::file_dropped(FileDropComponent* f, const File& file) {
    set_convolver(file);
}