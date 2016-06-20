#include "PlaybackPane.hpp"

#include "CommandIDs.h"

PlaybackPane::PlaybackPane(AudioFormatManager& audio_format_manager,
                           AudioTransportSource& audio_transport_source,
                           const File& file)
        : audio_format_manager(audio_format_manager)
        , transport_view_manager(audio_transport_source)
        , renderer(audio_format_manager, transport_view_manager)
        , ruler(transport_view_manager)
        , load_different_button("load different file")
        , follow_playback_button("follow playback")
        , bypass_button("enable convolution")
        , scroll_bar(false)
        , transport(audio_transport_source) {
    renderer.set_reader(audio_format_manager.createReaderFor(file), 0);

    for (auto i : {&follow_playback_button, &bypass_button}) {
        i->setWantsKeyboardFocus(false);
        i->triggerClick();
    }

    addAndMakeVisible(renderer);
    addAndMakeVisible(follow_playback_button);
    addAndMakeVisible(bypass_button);
    addAndMakeVisible(scroll_bar);
    addAndMakeVisible(transport);
    addAndMakeVisible(ruler);
    addAndMakeVisible(load_different_button);

    setWantsKeyboardFocus(true);
}

void PlaybackPane::reset_view() {
    transport_view_manager.reset_view();
}

void PlaybackPane::resized() {
    auto bounds = getLocalBounds();

    if (scroll_bar.isVisible()) {
        scroll_bar.setBounds(bounds.removeFromBottom(20));
    }

    auto top = bounds.removeFromTop(40);
    top.reduce(2, 2);
    ruler.setBounds(bounds.removeFromTop(20));
    renderer.setBounds(bounds);

    transport.setBounds(top.removeFromLeft(200));
    follow_playback_button.setBounds(top.removeFromRight(150));
    bypass_button.setBounds(top.removeFromRight(150));

    load_different_button.setBounds(top.removeFromLeft(150));
}

void PlaybackPane::max_range_changed(PlaybackViewManager* r,
                                     const Range<double>& range) {
    scroll_bar.setRangeLimits(range);
    resized();
}

void PlaybackPane::visible_range_changed(PlaybackViewManager* r,
                                         const Range<double>& range) {
    scroll_bar.setCurrentRange(range, dontSendNotification);
    resized();
}

void PlaybackPane::current_time_changed(PlaybackViewManager* r, double time) {
}

void PlaybackPane::scrollBarMoved(ScrollBar* s, double new_range_start) {
    if (s == &scroll_bar) {
        transport_view_manager.set_visible_range(scroll_bar.getCurrentRange(),
                                                 true);
    }
}

void PlaybackPane::buttonClicked(Button* b) {
    if (b == &follow_playback_button) {
        transport_view_manager.set_follow_playback(
                follow_playback_button.getToggleState());
    } else if (b == &bypass_button) {
        listener_list.call(&Listener::bypass_state_changed,
                           this,
                           bypass_button.getToggleState());
    } else if (b == &load_different_button) {
        FileChooser fc("open...",
                       File::nonexistent,
                       audio_format_manager.getWildcardForAllFormats());
        if (fc.browseForFileToOpen()) {
            listener_list.call(
                    &Listener::new_file_loaded, nullptr, fc.getResult());
        }
    }
}

void PlaybackPane::addListener(Listener* f) {
    listener_list.add(f);
}

void PlaybackPane::removeListener(Listener* f) {
    listener_list.remove(f);
}