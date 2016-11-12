#pragma once

#include "BasicPanel.h"
#include "HelpWindow.h"
#include "ValueWrapperEditableListBox.h"

/*
class MicrophoneListBox : public ValueWrapperListBox<model::Microphone> {
public:
    using ValueWrapperListBox<model::Microphone>::ValueWrapperListBox;

private:
    std::unique_ptr<Component> new_component_for_row(int row,
                                                     bool selected) override;
};

//----------------------------------------------------------------------------//

using MicrophoneEditableListBox =
        ValueWrapperEditableListBox<MicrophoneListBox>;

//----------------------------------------------------------------------------//

class SingleMicrophoneComponent : public BasicPanel {
public:
    SingleMicrophoneComponent(
            model::ValueWrapper<model::Microphone>& microphone);
};

//----------------------------------------------------------------------------//

class MicrophoneEditorPanel : public ListEditorPanel<MicrophoneEditableListBox>,
                              public SettableHelpPanelClient {
public:
    MicrophoneEditorPanel(
            model::ValueWrapper<util::aligned::vector<model::Microphone>>& microphones);

private:
    std::unique_ptr<Component> new_editor(
            model::ValueWrapper<value_type>& v) override;
};
*/
