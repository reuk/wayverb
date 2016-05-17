#pragma once

#include "ModelWrapper.hpp"

class MicrophoneListBox : public ListBox,
                          public ListBoxModel,
                          public ChangeListener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener& rhs) = default;
        Listener& operator=(const Listener& rhs) = default;
        Listener(Listener&& rhs) noexcept = default;
        Listener& operator=(Listener&& rhs) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void selectedRowsChanged(MicrophoneListBox* lb, int last) = 0;
    };

    MicrophoneListBox(
        model::ValueWrapper<config::MicrophoneModel>& microphone_model);

    int getNumRows() override;

    void paintListBoxItem(
        int row, Graphics& g, int w, int h, bool selected) override;

    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void selectedRowsChanged(int last) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    model::ValueWrapper<config::MicrophoneModel>& microphone_model;
    model::ChangeConnector microphone_connector{&microphone_model, this};

    ListenerList<Listener> listener_list;
};

//----------------------------------------------------------------------------//

class MicrophoneEditableListBox : public Component,
                                  public MicrophoneListBox::Listener,
                                  public TextButton::Listener,
                                  public ChangeListener {
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
        model::ValueWrapper<config::MicrophoneModel>& microphone_model);

    void buttonClicked(Button* b) override;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void selectedRowsChanged(MicrophoneListBox* lb, int last) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    void update_sub_button_enablement();

    model::ValueWrapper<config::MicrophoneModel>& microphone_model;
    model::ChangeConnector microphone_connector{&microphone_model, this};

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
        model::ValueWrapper<config::Microphone>& microphone);

    void resized() override;

private:
    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

class MicrophoneEditorPanel : public Component,
                              public MicrophoneEditableListBox::Listener {
public:
    MicrophoneEditorPanel(
        model::ValueWrapper<config::MicrophoneModel>& microphone_model);

    void resized() override;

    void selectedRowsChanged(MicrophoneEditableListBox* lb, int last) override;

private:
    model::ValueWrapper<config::MicrophoneModel>& microphone_model;

    MicrophoneEditableListBox microphone_list_box;
    model::Connector<MicrophoneEditableListBox> microphone_list_box_connector{
        &microphone_list_box, this};

    std::unique_ptr<SingleMicrophoneComponent> single_microphone;
};