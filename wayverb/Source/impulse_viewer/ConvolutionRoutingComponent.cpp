#include "ConvolutionRoutingComponent.hpp"

ImpulseRoutingComponent::ImpulseRoutingComponent(
        model::ValueWrapper<ImpulseRouting>& routing)
        : routing(routing) {
    addAndMakeVisible(channel_box);

    name_connector.trigger();
    channel_connector.trigger();
}

void ImpulseRoutingComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
    auto bounds = getLocalBounds().reduced(2);
    VisualiserLookAndFeel::matte_foreground_box(
            g,
            bounds,
            drag ? VisualiserLookAndFeel::emphasis : Colours::darkgrey);
    bounds.reduce(4, 4);
    g.setColour(Colours::lightgrey);
    g.drawText(routing.name.get(), bounds, Justification::topRight);
}

void ImpulseRoutingComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    bounds.removeFromTop(22);
    channel_box.setBounds(bounds.removeFromTop(25));
}

void ImpulseRoutingComponent::comboBoxChanged(ComboBox* cb) {
    if (cb == &channel_box) {
        //  TODO set channel properly
    }
}

void ImpulseRoutingComponent::mouseDrag(const MouseEvent& e) {
    if (auto dadc = DragAndDropContainer::findParentDragContainerFor(this)) {
        dadc->startDragging(var(), this);
    }
}

void ImpulseRoutingComponent::receive_broadcast(model::Broadcaster* b) {
    if (b == &routing.name) {
        repaint();
    } else if (b == &routing.channel) {
        //  TODO set channel properly
    }
}

bool ImpulseRoutingComponent::isInterestedInDragSource(
        const SourceDetails& details) {
    return dynamic_cast<CarrierRoutingComponent*>(
            details.sourceComponent.get());
}

void ImpulseRoutingComponent::itemDragEnter(const SourceDetails& details) {
    drag = true;
    repaint();
}

void ImpulseRoutingComponent::itemDragExit(const SourceDetails& details) {
    drag = false;
    repaint();
}

void ImpulseRoutingComponent::itemDropped(const SourceDetails& details) {
    drag = false;
    repaint();
}

CarrierRoutingComponent::CarrierRoutingComponent(
        model::ValueWrapper<CarrierRouting>& routing)
        : routing(routing) {
    name_connector.trigger();
    channel_connector.trigger();
}

void CarrierRoutingComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
    auto bounds = getLocalBounds().reduced(2);
    VisualiserLookAndFeel::matte_foreground_box(
            g,
            bounds,
            drag ? VisualiserLookAndFeel::emphasis : Colours::darkgrey);
    bounds.reduce(4, 4);
    g.setColour(Colours::lightgrey);
    g.drawText(routing.name.get(), bounds, Justification::topRight);
}

void CarrierRoutingComponent::receive_broadcast(model::Broadcaster* b) {
    if (b == &routing.name) {
        repaint();
    } else if (b == &routing.channel) {
        //  TODO set channel properly
    }
}

void CarrierRoutingComponent::mouseDrag(const MouseEvent& e) {
    if (auto dadc = DragAndDropContainer::findParentDragContainerFor(this)) {
        dadc->startDragging(var(), this);
    }
}

bool CarrierRoutingComponent::isInterestedInDragSource(
        const SourceDetails& details) {
    return dynamic_cast<ImpulseRoutingComponent*>(
            details.sourceComponent.get());
}

void CarrierRoutingComponent::itemDragEnter(const SourceDetails& details) {
    drag = true;
    repaint();
}

void CarrierRoutingComponent::itemDragExit(const SourceDetails& details) {
    drag = false;
    repaint();
}

void CarrierRoutingComponent::itemDropped(const SourceDetails& details) {
    drag = false;
    repaint();
}

//----------------------------------------------------------------------------//

Component* ImpulseRoutingListBox::refreshComponentForRow(int row,
                                                         bool selected,
                                                         Component* existing) {
    if (getNumRows() <= row) {
        delete existing;
        existing = nullptr;
    } else {
        if (existing) {
            delete existing;
            existing = nullptr;
        }
        if (!existing) {
            existing = new ImpulseRoutingComponent(model[row]);
        }

        auto& cmp = dynamic_cast<ImpulseRoutingComponent&>(*existing);
    }
    return existing;
}

Component* CarrierRoutingListBox::refreshComponentForRow(int row,
                                                         bool selected,
                                                         Component* existing) {
    if (getNumRows() <= row) {
        delete existing;
        existing = nullptr;
    } else {
        if (existing) {
            delete existing;
            existing = nullptr;
        }
        if (!existing) {
            existing = new CarrierRoutingComponent(model[row]);
        }

        auto& cmp = dynamic_cast<CarrierRoutingComponent&>(*existing);
    }
    return existing;
}

//----------------------------------------------------------------------------//

namespace {

std::vector<CarrierRouting> init_carrier_channels(int c, int output_channels) {
    std::vector<CarrierRouting> ret;
    for (auto i = 0u; i != c; ++i) {
        std::stringstream ss;
        ss << "Carrier channel " << i + 1;
        ret.emplace_back(CarrierRouting{
                ss.str(),
                std::vector<CarrierRouting::my_bool>(output_channels, 0)});
    }
    return ret;
}

std::vector<ImpulseRouting> init_hardware_channels(
        const AudioDeviceManager& m) {
    const auto active_channels =
            m.getCurrentAudioDevice()->getActiveOutputChannels();
    const auto names = m.getCurrentAudioDevice()->getOutputChannelNames();
    std::vector<ImpulseRouting> ret;
    for (auto i = 0u; i != names.size(); ++i) {
        if (active_channels[i]) {
            ret.emplace_back(ImpulseRouting{names[i].toStdString()});
        }
    }
    return ret;
}

}  // namespace

ConvolutionRoutingComponent::ConvolutionRoutingComponent(
        AudioDeviceManager& audio_device_manager, int carrier_channels)
        : audio_device_manager(audio_device_manager)
        , carrier_data(init_carrier_channels(
                  carrier_channels,
                  audio_device_manager.getCurrentAudioDevice()
                          ->getActiveOutputChannels()
                          .countNumberOfSetBits()))
        , hardware_data(init_hardware_channels(audio_device_manager)) {
    addAndMakeVisible(carrier_panel);
    addAndMakeVisible(hardware_panel);
}

void ConvolutionRoutingComponent::resized() {
    auto bounds = getLocalBounds().reduced(padding);

    auto panel_width = 200;
    carrier_panel.setBounds(
            bounds.removeFromLeft(panel_width)
                    .withHeight(carrier_panel.get_desired_height()));
    hardware_panel.setBounds(
            bounds.removeFromRight(panel_width)
                    .withHeight(hardware_panel.get_desired_height()));
}

void ConvolutionRoutingComponent::changeListenerCallback(
        ChangeBroadcaster* cb) {
}

int ConvolutionRoutingComponent::get_desired_height() const {
    return std::max(carrier_panel.get_desired_height(),
                    hardware_panel.get_desired_height()) +
           2 * padding;
}