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
    ImpulseRoutingComponent(model::ValueWrapper<ImpulseRouting>& routing,
                            int index);

    void paint(Graphics& g) override;
    void resized() override;
    void receive_broadcast(model::Broadcaster* b) override;
    void comboBoxChanged(ComboBox* cb) override;

    void mouseDrag(const MouseEvent& e) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

    model::ValueWrapper<ImpulseRouting>& routing;

    int get_index() const;

private:
    ComboBox channel_box;

    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};
    model::Connector<ComboBox> channel_box_connector{&channel_box, this};

    bool drag{false};
    int index{0};
};

class CarrierRoutingComponent : public Component,
                                public DragAndDropTarget,
                                public model::BroadcastListener {
public:
    CarrierRoutingComponent(model::ValueWrapper<CarrierRouting>& routing,
                            int index);

    void paint(Graphics& g) override;
    void receive_broadcast(model::Broadcaster* b) override;

    void mouseDrag(const MouseEvent& e) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

    model::ValueWrapper<CarrierRouting>& routing;

    int get_index() const;

private:
    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};

    bool drag{false};
    int index{0};
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
class RoutingPanel : public RoutingListBox {
public:
    RoutingPanel(std::string name,
                 typename RoutingListBox::model_type& model,
                 int row_height)
            : RoutingListBox(model) {
        auto header = new EmphasisLabel("", name);
        header->setSize(100, 20);
        header->setJustificationType(Justification::centred);
        this->setHeaderComponent(header);
        this->setRowHeight(row_height);
    }

    int get_desired_height() const {
        return this->get_full_content_height() +
               this->getHeaderComponent()->getHeight();
    }

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
};

using ImpulseRoutingPanel = RoutingPanel<ImpulseRoutingListBox>;
using CarrierRoutingPanel = RoutingPanel<CarrierRoutingListBox>;

//----------------------------------------------------------------------------//

class ConvolutionRoutingComponent : public Component,
                                    public ChangeListener,
                                    public ChangeBroadcaster,
                                    public model::BroadcastListener,
                                    public DragAndDropContainer {
public:

    ConvolutionRoutingComponent(AudioDeviceManager& audio_device_manager,
                                int carrier_channels);
    virtual ~ConvolutionRoutingComponent() noexcept;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;
    void receive_broadcast(model::Broadcaster* b) override;

    void dragOperationStarted() override;

    int get_desired_height() const;

    const std::vector<CarrierRouting>& get_carrier_routing() const;
    const std::vector<ImpulseRouting>& get_impulse_routing() const;

    static const int padding{20};

private:
    AudioDeviceManager& audio_device_manager;
    model::Connector<ChangeBroadcaster, ChangeListener>
            audio_device_manager_connector{&audio_device_manager, this};

    std::vector<CarrierRouting> carrier_data;
    std::vector<ImpulseRouting> impulse_data;

    model::ValueWrapper<std::vector<CarrierRouting>> carrier_model{
            nullptr, carrier_data};
    model::ValueWrapper<std::vector<ImpulseRouting>> impulse_model{
            nullptr, impulse_data};

    model::BroadcastConnector carrier_connector{&carrier_model, this};

    CarrierRoutingPanel carrier_panel{"carrier", carrier_model, 30};
    ImpulseRoutingPanel impulse_panel{"hardware", impulse_model, 60};

    class Connector;
    std::set<std::unique_ptr<Connector>> connectors;
};