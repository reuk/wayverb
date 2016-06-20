#pragma once

#include "AngularLookAndFeel.hpp"
#include "ConvolutionAudioSource.hpp"
#include "Routing.h"
#include "VUMeter.hpp"
#include "ValueWrapperListBox.hpp"

class ImpulseRoutingComponent : public Component,
                                public DragAndDropTarget,
                                public ComboBox::Listener,
                                public model::BroadcastListener,
                                public BufferReader {
public:
    ImpulseRoutingComponent(model::ValueWrapper<model::ImpulseRouting>& routing,
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

    model::ValueWrapper<model::ImpulseRouting>& routing;

    int get_index() const;

    void push_buffer(const float** input,
                     int num_channels,
                     int num_samples) override;

private:
    VUMeter meter;
    ComboBox channel_box;

    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};
    model::Connector<ComboBox> channel_box_connector{&channel_box, this};

    bool drag{false};
    int index{0};
};

class CarrierRoutingComponent : public Component,
                                public DragAndDropTarget,
                                public model::BroadcastListener,
                                public BufferReader {
public:
    CarrierRoutingComponent(model::ValueWrapper<model::CarrierRouting>& routing,
                            int index);

    void paint(Graphics& g) override;
    void resized() override;
    void receive_broadcast(model::Broadcaster* b) override;

    void mouseDrag(const MouseEvent& e) override;

    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

    model::ValueWrapper<model::CarrierRouting>& routing;

    int get_index() const;

    void push_buffer(const float** input,
                     int num_channels,
                     int num_samples) override;

private:
    VUMeter meter;

    model::BroadcastConnector name_connector{&routing.name, this};
    model::BroadcastConnector channel_connector{&routing.channel, this};

    bool drag{false};
    int index{0};
};

//----------------------------------------------------------------------------//

class ImpulseRoutingListBox : public ValueWrapperListBox<model::ImpulseRouting>,
                              public BufferReader {
public:
    using ValueWrapperListBox<model::ImpulseRouting>::ValueWrapperListBox;
    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;

    template <typename T>
    void set_to(T&& t) {
        to = std::forward<T>(t);
        updateContent();
    }

    void push_buffer(const float** input,
                     int num_channels,
                     int num_samples) override;

private:
    std::vector<std::string> to;
};

class CarrierRoutingListBox : public ValueWrapperListBox<model::CarrierRouting>,
                              public BufferReader {
public:
    using ValueWrapperListBox<model::CarrierRouting>::ValueWrapperListBox;
    Component* refreshComponentForRow(int row,
                                      bool selected,
                                      Component* existing) override;

    void push_buffer(const float** input,
                     int num_channels,
                     int num_samples) override;
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
            AngularLookAndFeel::matte_foreground_box(
                    g, getLocalBounds(), AngularLookAndFeel::emphasis);
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
                                    public ConvolutionAudioSource::Listener {
public:
    ConvolutionRoutingComponent(AudioDeviceManager& audio_device_manager,
                                int carrier_channels);
    virtual ~ConvolutionRoutingComponent() noexcept;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    int get_desired_height() const;

    const std::vector<model::CarrierRouting>& get_carrier_routing() const;
    const std::vector<model::ImpulseRouting>& get_impulse_routing() const;

    void carrier_signal_in(ConvolutionAudioSource*,
                           const float** input,
                           int num_channels,
                           int num_samples) override;
    void impulse_signal_in(ConvolutionAudioSource*,
                           const float** input,
                           int num_channels,
                           int num_samples) override;

private:
    AudioDeviceManager& audio_device_manager;
    int carrier_channels;
    model::Connector<ChangeBroadcaster, ChangeListener>
            audio_device_manager_connector{&audio_device_manager, this};

    class Impl;
    std::unique_ptr<Impl> pimpl;
};