#pragma once

#include "HelpWindow.hpp"
#include "ValueWrapperListBox.hpp"

class MicrophoneListBox : public ValueWrapperListBox<model::Microphone> {
public:
    using ValueWrapperListBox<model::Microphone>::ValueWrapperListBox;
    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;
};

//----------------------------------------------------------------------------//

class MicrophoneEditableListBox : public Component,
                                  public MicrophoneListBox::Listener,
                                  public TextButton::Listener,
                                  public model::BroadcastListener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener& rhs) = default;
        Listener& operator=(const Listener& rhs) = default;
        Listener(Listener&& rhs) noexcept = default;
        Listener& operator=(Listener&& rhs) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void selectedRowsChanged(MicrophoneEditableListBox* lb,
                                         int last) = 0;
    };

    MicrophoneEditableListBox(
            model::ValueWrapper<std::vector<model::Microphone>>& microphones);

    void buttonClicked(Button* b) override;

    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void selectRow(int row);
    void selectedRowsChanged(ValueWrapperListBox<model::Microphone>* lb,
                             int last) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    void update_sub_button_enablement();

    model::ValueWrapper<std::vector<model::Microphone>>& microphones;
    model::BroadcastConnector microphones_connector{&microphones, this};

    MicrophoneListBox microphone_list_box;
    model::Connector<MicrophoneListBox> microphone_list_box_connector{
            &microphone_list_box, this};

    TextButton add_button{"+"};
    model::Connector<TextButton> add_button_connector{&add_button, this};

    TextButton sub_button{"-"};
    model::Connector<TextButton> sub_button_connector{&sub_button, this};

    ListenerList<Listener> listener_list;
};

//----------------------------------------------------------------------------//

class SingleMicrophoneComponent : public Component {
public:
    SingleMicrophoneComponent(
            model::ValueWrapper<model::Microphone>& microphone);

    void resized() override;

    int getTotalContentHeight() const;

private:
    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

class MicrophoneEditorPanel : public Component,
                              public MicrophoneEditableListBox::Listener,
                              public SettableHelpPanelClient {
public:
    MicrophoneEditorPanel(
            model::ValueWrapper<std::vector<model::Microphone>>& microphones);

    void resized() override;

    void selectedRowsChanged(MicrophoneEditableListBox* lb, int last) override;

private:
    model::ValueWrapper<std::vector<model::Microphone>>& microphones;

    MicrophoneEditableListBox microphone_list_box;
    model::Connector<MicrophoneEditableListBox> microphone_list_box_connector{
            &microphone_list_box, this};

    std::unique_ptr<SingleMicrophoneComponent> single_microphone;
};