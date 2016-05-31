#include "MicrophoneEditor.hpp"
#include "DirectionEditor.hpp"

#include "PolarPatternDisplay.hpp"

#include "Vec3Editor.hpp"

MicrophoneListBox::MicrophoneListBox(
    model::ValueWrapper<config::MicrophoneModel>& microphone_model)
        : microphone_model(microphone_model) {
    //  don't use this with an empy list plz
    assert(!microphone_model.microphones.empty());
    setModel(this);
}

int MicrophoneListBox::getNumRows() {
    return microphone_model.microphones.size();
}

void MicrophoneListBox::paintListBoxItem(
    int row, Graphics& g, int w, int h, bool selected) {
}

Component* MicrophoneListBox::refreshComponentForRow(int row,
                                                     bool selected,
                                                     Component* existing) {
    if (getNumRows() <= row) {
        delete existing;
        existing = nullptr;
    } else {
        if (!existing) {
            existing = new Label();
        }

        auto& label = dynamic_cast<Label&>(*existing);

        std::stringstream ss;
        ss << "microphone " << row + 1;
        label.setText(ss.str(), dontSendNotification);
        label.setColour(Label::ColourIds::textColourId, Colours::black);
        label.setColour(Label::ColourIds::textColourId,
                        selected ? Colours::lightgrey : Colours::black);
        label.setColour(
            Label::ColourIds::backgroundColourId,
            selected ? Colours::darkgrey : Colours::transparentWhite);

        label.setInterceptsMouseClicks(false, false);
    }
    return existing;
}

void MicrophoneListBox::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &microphone_model) {
        updateContent();
    }
}

void MicrophoneListBox::selectedRowsChanged(int last) {
    listener_list.call(&Listener::selectedRowsChanged, this, last);
}

void MicrophoneListBox::addListener(Listener* l) {
    listener_list.add(l);
}
void MicrophoneListBox::removeListener(Listener* l) {
    listener_list.remove(l);
}

//----------------------------------------------------------------------------//

MicrophoneEditableListBox::MicrophoneEditableListBox(
    model::ValueWrapper<config::MicrophoneModel>& microphone_model)
        : microphone_model(microphone_model)
        , microphone_list_box(microphone_model) {
    addAndMakeVisible(microphone_list_box);
    addAndMakeVisible(add_button);
    addAndMakeVisible(sub_button);

    receive_broadcast(&microphone_model);
}

void MicrophoneEditableListBox::buttonClicked(Button* b) {
    if (b == &add_button) {
        microphone_model.microphones.push_back(
            config::Microphone{glm::vec3{0, 0, 1}, 0.5});
        microphone_list_box.selectRow(microphone_model.microphones.size() - 1);
    } else if (b == &sub_button) {
        assert(0 <= microphone_list_box.getSelectedRow());
        microphone_model.microphones.erase(
            microphone_list_box.getSelectedRow());
        microphone_list_box.deselectAllRows();
    }
}

void MicrophoneEditableListBox::resized() {
    auto button_height = 25;
    auto bounds = getLocalBounds();

    microphone_list_box.setBounds(bounds.withTrimmedBottom(button_height));

    auto button_area = bounds.removeFromBottom(button_height).reduced(0, 2);
    sub_button.setBounds(button_area.withTrimmedRight(getWidth() / 2 + 1));
    add_button.setBounds(button_area.withTrimmedLeft(getWidth() / 2 + 1));
}

void MicrophoneEditableListBox::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &microphone_model) {
        update_sub_button_enablement();
    }
}

void MicrophoneEditableListBox::selectedRowsChanged(MicrophoneListBox* lb,
                                                    int last) {
    listener_list.call(&Listener::selectedRowsChanged, this, last);
    update_sub_button_enablement();
}

void MicrophoneEditableListBox::addListener(Listener* l) {
    listener_list.add(l);
}
void MicrophoneEditableListBox::removeListener(Listener* l) {
    listener_list.remove(l);
}

void MicrophoneEditableListBox::update_sub_button_enablement() {
    auto more_than_one_entry = 1 < microphone_model.microphones.size();
    auto row_selected = microphone_list_box.getSelectedRow() != -1;
    sub_button.setEnabled(more_than_one_entry && row_selected);
}

//----------------------------------------------------------------------------//

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

SingleMicrophoneComponent::SingleMicrophoneComponent(
    model::ValueWrapper<config::Microphone>& microphone,
    model::ValueWrapper<glm::vec3>& mic_position) {
    property_panel.addProperties(
        {new DirectionProperty(microphone.facing, mic_position)});
    property_panel.addProperties(
        {new NumberProperty<float>("shape", microphone.shape, 0, 1)});
    property_panel.addProperties({new PolarPatternProperty(microphone.shape)});

    addAndMakeVisible(property_panel);
}

void SingleMicrophoneComponent::resized() {
    property_panel.setBounds(getLocalBounds());
}

//----------------------------------------------------------------------------//

MicrophoneEditorPanel::MicrophoneEditorPanel(
    model::ValueWrapper<config::MicrophoneModel>& microphone_model,
    model::ValueWrapper<glm::vec3>& mic_position)
        : microphone_model(microphone_model)
        , mic_position(mic_position)
        , microphone_list_box(microphone_model) {
    set_help("microphones configurator",
             "Simulated microphones behave like a collection of superimposed "
             "microphone capsules. Add and remove capsules using the list on "
             "the left, and configure individual capsules using the options on "
             "the right.");
    addAndMakeVisible(microphone_list_box);

    microphone_model.notify();

    setSize(500, 350);
}

void MicrophoneEditorPanel::resized() {
    auto bounds = getLocalBounds();
    microphone_list_box.setBounds(bounds.removeFromLeft(150));
    if (single_microphone) {
        single_microphone->setBounds(bounds);
    }
}

void MicrophoneEditorPanel::selectedRowsChanged(MicrophoneEditableListBox* lb,
                                                int last) {
    assert(last < static_cast<int>(microphone_model.microphones.size()));
    if (0 <= last) {
        single_microphone = std::make_unique<SingleMicrophoneComponent>(
            microphone_model.microphones[last], mic_position);
        addAndMakeVisible(*single_microphone);
        resized();
    } else {
        single_microphone = nullptr;
    }
}