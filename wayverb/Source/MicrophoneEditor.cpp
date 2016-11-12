#include "MicrophoneEditor.h"
#include "DirectionEditor.h"
#include "PolarPatternDisplay.h"
#include "Vec3Editor.h"

/*
std::unique_ptr<Component> MicrophoneListBox::new_component_for_row(
        int row, bool selected) {
    auto ret = std::make_unique<Label>();

    std::stringstream ss;
    ss << "microphone " << row + 1;
    ret->setText(ss.str(), dontSendNotification);
    configure_selectable_label(*ret, selected);
    return std::unique_ptr<Component>(std::move(ret));
}

//----------------------------------------------------------------------------//

namespace {
class PolarPatternProperty : public PropertyComponent {
public:
    PolarPatternProperty(model::ValueWrapper<float>& shape)
            : PropertyComponent("polar pattern", 120)
            , display(shape) {
        addAndMakeVisible(display);
    }

    void refresh() override {
    }

private:
    PolarPatternDisplay display;
};
}  // namespace

SingleMicrophoneComponent::SingleMicrophoneComponent(
        model::ValueWrapper<model::Microphone>& microphone)
        : BasicPanel([&microphone](auto& panel) {
            panel.addProperties({new DirectionProperty(microphone.pointer)});
            panel.addProperties({new NumberProperty<float>(
                    "shape", microphone.shape, 0, 1)});
            panel.addProperties({new PolarPatternProperty(microphone.shape)});
        }) {
}

//----------------------------------------------------------------------------//

MicrophoneEditorPanel::MicrophoneEditorPanel(
        model::ValueWrapper<aligned::vector<model::Microphone>>& model)
        : ListEditorPanel<MicrophoneEditableListBox>(model) {
    set_help("microphones configurator",
             "Simulated microphones behave like a collection of superimposed "
             "microphone capsules. Add and remove capsules using the list on "
             "the left, and configure individual capsules using the options on "
             "the right.");
}

std::unique_ptr<Component> MicrophoneEditorPanel::new_editor(
        model::ValueWrapper<value_type>& v) {
    return std::make_unique<SingleMicrophoneComponent>(v);
}
*/
