#include "ConvolutionViewer.hpp"

#include "ConvolutionRoutingComponent.hpp"
#include "PlaybackPane.hpp"

class ConvolutionViewer : public Component {
public:
    ConvolutionViewer(AudioDeviceManager& audio_device_manager,
                      AudioFormatManager& audio_format_manager,
                      const File& f)
            : audio_device_manager(audio_device_manager)
            , audio_format_manager(audio_format_manager)
            , audio_format_reader_source(
                      audio_format_manager.createReaderFor(f), true)
            , playback_pane(audio_device_manager,
                            audio_format_manager,
                            audio_format_reader_source,
                            f)
            , resizer_bar(&layout, 1, false)
            , convolution_routing(
                      audio_device_manager,
                      audio_format_reader_source.getAudioFormatReader()
                              ->numChannels) {
        addAndMakeVisible(playback_pane);
        addAndMakeVisible(resizer_bar);
        addAndMakeVisible(viewport);

        layout.setItemLayout(0, 200, 10000, 200);
        layout.setItemLayout(1, 4, 4, 4);
        layout.setItemLayout(2, 1, 10000, 200);

        viewport.setViewedComponent(&convolution_routing, false);
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

    void addListener(FileDropListener* l) {
        playback_pane.addListener(l);
    }
    void removeListener(FileDropListener* l) {
        playback_pane.removeListener(l);
    }

private:
    AudioDeviceManager& audio_device_manager;
    AudioFormatManager& audio_format_manager;
    AudioFormatReaderSource audio_format_reader_source;

    StretchableLayoutManager layout;
    PlaybackPane playback_pane;
    StretchableLayoutResizerBar resizer_bar;
    Viewport viewport;
    ConvolutionRoutingComponent convolution_routing;
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

void ConvolutionLoader::file_dropped(Component* f, const File& file) {
    set_convolver(file);
}