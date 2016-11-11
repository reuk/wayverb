#include "SourcesEditor.hpp"

#include "DirectionEditor.hpp"
#include "MicrophoneEditor.hpp"
#include "Vec3Editor.hpp"

/*
std::unique_ptr<Component> SourcesListBox::new_component_for_row(
        int row, bool selected) {
    auto ret = std::make_unique<Label>();
    std::stringstream ss;
    ss << "source " << row + 1;
    ret->setText(ss.str(), dontSendNotification);
    configure_selectable_label(*ret, selected);
    return std::unique_ptr<Component>(std::move(ret));
}

//----------------------------------------------------------------------------//

class SingleSourceComponent : public BasicPanel {
public:
    SingleSourceComponent()
            : BasicPanel([](auto& panel) {
                panel.addProperties({new Vec3Property("source position")});
            }) {}
};

std::unique_ptr<Component> SourcesEditorPanel::new_editor() {
    return std::make_unique<SingleSourceComponent>();
}

//----------------------------------------------------------------------------//

std::unique_ptr<Component> ReceiversListBox::new_component_for_row(
        int row, bool selected) {
    auto ret = std::make_unique<Label>();
    std::stringstream ss;
    ss << "receiver " << row + 1;
    ret->setText(ss.str(), dontSendNotification);
    configure_selectable_label(*ret, selected);
    return std::unique_ptr<Component>(std::move(ret));
}

//----------------------------------------------------------------------------//

class HrtfModelComponent : public BasicPanel, public SettableHelpPanelClient {
public:
    HrtfModelComponent()
            : BasicPanel([](auto& panel) {
                panel.addProperties({new DirectionProperty()});
            }) {
        set_help("hrtf configurator",
                 "There's only one option, which allows you to choose the "
                 "direction that the virtual head should be facing.");
    }
};

//----------------------------------------------------------------------------//

class ReceiverSettingsComponent : public TabbedComponent {
public:
    ReceiverSettingsComponent()
            : TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop) {
    }
};

class ReceiverSettingsProperty : public PropertyComponent {
public:
    ReceiverSettingsProperty()
            : PropertyComponent("receiver settings", 300)
            , cmp() {
        addAndMakeVisible(cmp);
    }

    void refresh() override {}

private:
    ReceiverSettingsComponent cmp;
};

class SingleReceiverComponent : public BasicPanel {
public:
    SingleReceiverComponent()
            : BasicPanel([](auto& panel) {
                panel.addProperties({new Vec3Property("receiver position")});
                panel.addProperties({new ReceiverSettingsProperty()});
            }) {}
};

ReceiversEditorPanel::ReceiversEditorPanel()
        : ListEditorPanel<ReceiversEditableListBox>() {}

std::unique_ptr<Component> ReceiversEditorPanel::new_editor() {
    return std::make_unique<SingleReceiverComponent>();
}
*/
