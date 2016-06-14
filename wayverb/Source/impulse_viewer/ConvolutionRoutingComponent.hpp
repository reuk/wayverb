#pragma once

#include "ValueWrapperListBox.hpp"
#include "VisualiserLookAndFeel.hpp"

struct ImpulseRouting {
    std::string name{""};
    int channel{-1};  // -1 for none
};

template <>
class model::ValueWrapper<ImpulseRouting>
        : public StructWrapper<ImpulseRouting, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&name, &channel}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(channel);
};

struct CarrierRouting {
    using my_bool = int8_t;
    std::string name{""};
    std::vector<my_bool> channel;
};

template <>
class model::ValueWrapper<CarrierRouting>
        : public StructWrapper<CarrierRouting, 2> {
public:
    using struct_wrapper::StructWrapper;
    member_array get_members() override {
        return {{&name, &channel}};
    }
    MODEL_FIELD_DEFINITION(name);
    MODEL_FIELD_DEFINITION(channel);
};

//----------------------------------------------------------------------------//

class ImpulseRoutingComponent : public Component,
                                public DragAndDropTarget,
                                public ComboBox::Listener,
                                public model::BroadcastListener {
public:
    ImpulseRoutingComponent(model::ValueWrapper<ImpulseRouting>& routing);

    void paint(Graphics& g) override;
    void resized() override;
    void receive_broadcast(model::Broadcaster* b) override;
    void comboBoxChanged(ComboBox* cb) override;

    void mouseDrag(const MouseEvent& e) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

private:
    model::ValueWrapper<ImpulseRouting>& routing;
    ComboBox channel_box;

    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};
    model::Connector<ComboBox> channel_box_connector{&channel_box, this};

    bool drag{false};
};

class CarrierRoutingComponent : public Component,
                                public DragAndDropTarget,
                                public model::BroadcastListener {
public:
    CarrierRoutingComponent(model::ValueWrapper<CarrierRouting>& routing);

    void paint(Graphics& g) override;
    void receive_broadcast(model::Broadcaster* b) override;

    void mouseDrag(const MouseEvent& e) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

private:
    model::ValueWrapper<CarrierRouting>& routing;

    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};

    bool drag{false};
};

//----------------------------------------------------------------------------//

class ImpulseRoutingListBox : public ValueWrapperListBox<ImpulseRouting> {
public:
    using ValueWrapperListBox<ImpulseRouting>::ValueWrapperListBox;
    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;

    template <typename T>
    void set_to(T&& t) {
        to = std::forward<T>(t);
        updateContent();
    }

private:
    std::vector<std::string> to;
};

class CarrierRoutingListBox : public ValueWrapperListBox<CarrierRouting> {
public:
    using ValueWrapperListBox<CarrierRouting>::ValueWrapperListBox;
    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;
};

//----------------------------------------------------------------------------//

template <typename RoutingListBox>
class RoutingPanel : public Component {
public:
    RoutingPanel(std::string name,
                 typename RoutingListBox::model_type& model,
                 int row_height)
            : label("", name)
            , routing_list_box(model) {
        label.setJustificationType(Justification::centred);
        routing_list_box.setRowHeight(row_height);

        addAndMakeVisible(label);
        addAndMakeVisible(routing_list_box);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(label_height));
        routing_list_box.setBounds(bounds);
    }

    int get_desired_height() const {
        return routing_list_box.get_full_content_height() + label_height;
    }

    static const int label_height{20};

private:
    class EmphasisLabel : public Label {
    public:
        using Label::Label;
        void paint(Graphics& g) override {
            VisualiserLookAndFeel::matte_foreground_box(
                    g, getLocalBounds(), VisualiserLookAndFeel::emphasis);
            Label::paint(g);
        }
    };
    EmphasisLabel label;
    RoutingListBox routing_list_box;
};

class ImpulseRoutingPanel : public RoutingPanel<ImpulseRoutingListBox> {
public:
    using RoutingPanel<ImpulseRoutingListBox>::RoutingPanel;
};

class CarrierRoutingPanel : public RoutingPanel<CarrierRoutingListBox> {
public:
    using RoutingPanel<CarrierRoutingListBox>::RoutingPanel;
};

//----------------------------------------------------------------------------//

class ConvolutionRoutingComponent : public Component,
                                    public ChangeListener,
                                    public DragAndDropContainer {
public:
    ConvolutionRoutingComponent(AudioDeviceManager& audio_device_manager,
                                int carrier_channels);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    int get_desired_height() const;

    static const int padding{20};

private:
    AudioDeviceManager& audio_device_manager;
    model::Connector<ChangeBroadcaster, ChangeListener>
            audio_device_manager_connector{&audio_device_manager, this};

    std::vector<CarrierRouting> carrier_data;
    std::vector<ImpulseRouting> hardware_data;

    model::ValueWrapper<std::vector<CarrierRouting>> carrier_model{
            nullptr, carrier_data};
    model::ValueWrapper<std::vector<ImpulseRouting>> hardware_model{
            nullptr, hardware_data};

    CarrierRoutingPanel carrier_panel{"carrier", carrier_model, 30};
    ImpulseRoutingPanel hardware_panel{"hardware", hardware_model, 60};
};