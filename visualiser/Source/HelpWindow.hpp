#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//  Help panel - behaves almost exactly the same as JUCE tooltips,
//  but persistent, and in their own window

class HelpPanel : public Component, private Timer {
public:
    struct PanelContents final {
        explicit PanelContents(const std::string& title = "",
                               const std::string& text = "");

        std::string title;
        std::string text;

        bool operator!=(const PanelContents& rhs) const;
        bool operator==(const PanelContents& rhs) const;

        auto tie() const {
            return std::tie(title, text);
        }
    };

    HelpPanel();

    void set_text(const PanelContents& contents);

private:
    Point<float> last_mouse_pos;
    Component* last_component_under_mouse;

    PanelContents showing;
    PanelContents last;

    Label title;
    Label text;

    unsigned int last_comp_change_time{0};

    void resized() override;

    void timerCallback() override;

    static PanelContents get_help_for(Component*);
};

class HelpPanelClient {
public:
    HelpPanelClient() = default;
    HelpPanelClient(const HelpPanelClient&) = default;
    HelpPanelClient& operator=(const HelpPanelClient&) = default;
    HelpPanelClient(HelpPanelClient&&) noexcept = default;
    HelpPanelClient& operator=(HelpPanelClient&&) noexcept = default;
    virtual ~HelpPanelClient() noexcept = default;

    virtual HelpPanel::PanelContents get_help() const = 0;
};

class SettableHelpPanelClient : public HelpPanelClient {
public:
    HelpPanel::PanelContents get_help() const override;

    void set_help(const std::string& title, const std::string& text);
    void set_help(const HelpPanel::PanelContents& u);

private:
    HelpPanel::PanelContents contents;
};