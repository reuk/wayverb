#include "HelpWindow.h"

HelpPanel::PanelContents::PanelContents(const std::string& title,
                                        const std::string& text)
        : title(title)
        , text(text) {}

bool HelpPanel::PanelContents::operator!=(const PanelContents& rhs) const {
    return tie() != rhs.tie();
}

bool HelpPanel::PanelContents::operator==(const PanelContents& rhs) const {
    return tie() == rhs.tie();
}

HelpPanel::HelpPanel() {
    if (Desktop::getInstance().getMainMouseSource().canHover()) {
        startTimer(123);
    }

    title.setFont(Font(20));
    title.setJustificationType(Justification::centred);
    text.setJustificationType(Justification::topLeft);
    text.setMinimumHorizontalScale(1);

    addAndMakeVisible(title);
    addAndMakeVisible(text);
}

void HelpPanel::set_text(const PanelContents& tip) {
    if (showing != tip) {
        showing = tip;

        title.setText(tip.title, dontSendNotification);
        text.setText(tip.text, dontSendNotification);
    }
}

void HelpPanel::resized() {
    auto title_height = 30;
    auto bounds = getLocalBounds();
    title.setBounds(bounds.removeFromTop(title_height));
    text.setBounds(bounds);
}

void HelpPanel::timerCallback() {
    auto& desktop = Desktop::getInstance();
    MouseInputSource mouse_source(desktop.getMainMouseSource());
    auto now = Time::getApproximateMillisecondCounter();

    auto new_comp = mouse_source.isMouse()
                            ? mouse_source.getComponentUnderMouse()
                            : nullptr;
    auto new_tip = get_help_for(new_comp);
    auto tip_changed =
            (new_tip != last || new_comp != last_component_under_mouse);
    last_component_under_mouse = new_comp;
    last = new_tip;

    auto mouse_pos = mouse_source.getScreenPosition();
    auto mouse_moved_quickly = mouse_pos.getDistanceFrom(last_mouse_pos) > 12;
    last_mouse_pos = mouse_pos;

    if (tip_changed || mouse_moved_quickly) {
        last_comp_change_time = now;
    }

    // if there isn't currently a tip, but one is needed, only let it
    // appear after a timeout..
    if (new_tip != PanelContents{} && new_tip != showing &&
        now > last_comp_change_time) {
        set_text(new_tip);
    }
}

HelpPanel::PanelContents HelpPanel::get_help_for(Component* c) {
    if (c != nullptr && Process::isForegroundProcess()) {
        if (auto client = dynamic_cast<HelpPanelClient*>(c)) {
            if (!c->isCurrentlyBlockedByAnotherModalComponent()) {
                return client->get_help();
            }
        } else {
            return get_help_for(c->getParentComponent());
        }
    }
    return HelpPanel::PanelContents{
            "no object hovered", "hover over a control for more information"};
}

//----------------------------------------------------------------------------//

HelpPanel::PanelContents SettableHelpPanelClient::get_help() const {
    return contents;
}

void SettableHelpPanelClient::set_help(const std::string& title,
                                       const std::string& text) {
    set_help(HelpPanel::PanelContents(title, text));
}
void SettableHelpPanelClient::set_help(const HelpPanel::PanelContents& u) {
    contents = u;
}
