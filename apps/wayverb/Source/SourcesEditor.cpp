#include "SourcesEditor.hpp"

#include "DirectionEditor.hpp"
#include "MicrophoneEditor.hpp"
#include "Vec3Editor.hpp"

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
    SingleSourceComponent(model::ValueWrapper<glm::vec3>& source,
                          const box& aabb)
            : BasicPanel([&source, &aabb](auto& panel) {
                panel.addProperties({new Vec3Property("source position",
                                                      source,
                                                      aabb.get_c0(),
                                                      aabb.get_c1())});
            }) {}
};

SourcesEditorPanel::SourcesEditorPanel(model_type& model, const box& aabb)
        : ListEditorPanel<SourcesEditableListBox>(model)
        , aabb(aabb) {}

std::unique_ptr<Component> SourcesEditorPanel::new_editor(
        model::ValueWrapper<value_type>& v) {
    return std::make_unique<SingleSourceComponent>(v, aabb);
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
    HrtfModelComponent(model::ValueWrapper<model::Pointer>& model)
            : BasicPanel([&model](auto& panel) {
                panel.addProperties({new DirectionProperty(model)});
            }) {
        set_help("hrtf configurator",
                 "There's only one option, which allows you to choose the "
                 "direction that the virtual head should be facing.");
    }
};

//----------------------------------------------------------------------------//

class ReceiverSettingsComponent : public TabbedComponent {
public:
    ReceiverSettingsComponent(
            model::ValueWrapper<model::ReceiverSettings>& model)
            : TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
            , model(model) {
        addTab("microphones",
               Colours::darkgrey,
               new MicrophoneEditorPanel(model.microphones),
               true);
        addTab("hrtf",
               Colours::darkgrey,
               new HrtfModelComponent(model.hrtf),
               true);
    }

private:
    model::ValueWrapper<model::ReceiverSettings>& model;
};

class ReceiverSettingsProperty : public PropertyComponent {
public:
    ReceiverSettingsProperty(
            model::ValueWrapper<model::ReceiverSettings>& model)
            : PropertyComponent("receiver settings", 300)
            , cmp(model) {
        addAndMakeVisible(cmp);
    }

    void refresh() override {}

private:
    ReceiverSettingsComponent cmp;
};

class SingleReceiverComponent : public BasicPanel {
public:
    SingleReceiverComponent(
            model::ValueWrapper<model::ReceiverSettings>& receiver,
            const box& aabb)
            : BasicPanel([&receiver, &aabb](auto& panel) {
                panel.addProperties({new Vec3Property("receiver position",
                                                      receiver.position,
                                                      aabb.get_c0(),
                                                      aabb.get_c1())});
                panel.addProperties({new ReceiverSettingsProperty(receiver)});
            }) {}
};

ReceiversEditorPanel::ReceiversEditorPanel(model_type& model, const box& aabb)
        : ListEditorPanel<ReceiversEditableListBox>(model)
        , aabb(aabb) {}

std::unique_ptr<Component> ReceiversEditorPanel::new_editor(
        model::ValueWrapper<value_type>& v) {
    return std::make_unique<SingleReceiverComponent>(v, aabb);
}