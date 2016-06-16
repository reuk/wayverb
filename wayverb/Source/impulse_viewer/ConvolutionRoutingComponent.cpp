#include "ConvolutionRoutingComponent.hpp"

ImpulseRoutingComponent::ImpulseRoutingComponent(
        model::ValueWrapper<model::ImpulseRouting>& routing, int index)
        : routing(routing)
        , meter(index)
        , index(index) {
    addAndMakeVisible(meter);
    addAndMakeVisible(channel_box);

    name_connector.trigger();
    channel_connector.trigger();
}

void ImpulseRoutingComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
    auto bounds = getLocalBounds().reduced(2);
    VisualiserLookAndFeel::matte_box(
            g,
            bounds,
            false,
            drag ? VisualiserLookAndFeel::emphasis : Colours::darkgrey);
    bounds.reduce(4, 4);
    g.setColour(Colours::lightgrey);
    g.drawText(routing.name.get(), bounds, Justification::topRight);
}

void ImpulseRoutingComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    bounds.removeFromTop(22);
    meter.setBounds(bounds.removeFromBottom(2));
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
    auto dropped = dynamic_cast<CarrierRoutingComponent*>(
            details.sourceComponent.get());

    assert(dropped);
    assert(0 <= index);
    assert(index < dropped->routing.channel.size());

    dropped->routing.channel[index].set(1);

    drag = false;
    repaint();
}

int ImpulseRoutingComponent::get_index() const {
    return index;
}

void ImpulseRoutingComponent::push_buffer(const float** input,
                                          int num_channels,
                                          int num_samples) {
    meter.push_buffer(input, num_channels, num_samples);
}

CarrierRoutingComponent::CarrierRoutingComponent(
        model::ValueWrapper<model::CarrierRouting>& routing, int index)
        : routing(routing)
        , meter(index)
        , index(index) {
    addAndMakeVisible(meter);

    name_connector.trigger();
    channel_connector.trigger();
}

void CarrierRoutingComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
    auto bounds = getLocalBounds().reduced(2);
    VisualiserLookAndFeel::matte_box(
            g,
            bounds,
            false,
            drag ? VisualiserLookAndFeel::emphasis : Colours::darkgrey);
    bounds.reduce(4, 4);
    g.setColour(Colours::lightgrey);
    g.drawText(routing.name.get(), bounds, Justification::topRight);
}

void CarrierRoutingComponent::resized() {
    auto bounds = getLocalBounds().reduced(4);
    meter.setBounds(bounds.removeFromBottom(2));
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
    auto dropped = dynamic_cast<ImpulseRoutingComponent*>(
            details.sourceComponent.get());

    assert(dropped);
    assert(0 <= dropped->get_index());
    assert(dropped->get_index() < routing.channel.size());

    routing.channel[dropped->get_index()].set(1);

    drag = false;
    repaint();
}

int CarrierRoutingComponent::get_index() const {
    return index;
}

void CarrierRoutingComponent::push_buffer(const float** input,
                                          int num_channels,
                                          int num_samples) {
    meter.push_buffer(input, num_channels, num_samples);
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
            existing = new ImpulseRoutingComponent(get_model()[row], row);
        }

        //        auto& cmp = dynamic_cast<ImpulseRoutingComponent&>(*existing);
    }
    return existing;
}

void ImpulseRoutingListBox::push_buffer(const float** input,
                                        int num_channels,
                                        int num_samples) {
    for (auto i = 0u; i != getNumRows(); ++i) {
        auto cmp = dynamic_cast<ImpulseRoutingComponent*>(
                getComponentForRowNumber(i));
        if (cmp) {
            cmp->push_buffer(input, num_channels, num_samples);
        }
    }
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
            existing = new CarrierRoutingComponent(get_model()[row], row);
        }

        //        auto& cmp = dynamic_cast<CarrierRoutingComponent&>(*existing);
    }
    return existing;
}

void CarrierRoutingListBox::push_buffer(const float** input,
                                        int num_channels,
                                        int num_samples) {
    for (auto i = 0u; i != getNumRows(); ++i) {
        auto cmp = dynamic_cast<CarrierRoutingComponent*>(
                getComponentForRowNumber(i));
        if (cmp) {
            cmp->push_buffer(input, num_channels, num_samples);
        }
    }
}

//----------------------------------------------------------------------------//

namespace {

std::vector<model::CarrierRouting> init_carrier_channels(int c,
                                                         int output_channels) {
    std::vector<model::CarrierRouting> ret;
    for (auto i = 0u; i != c; ++i) {
        std::stringstream ss;
        ss << "Carrier channel " << i + 1;
        ret.emplace_back(model::CarrierRouting{
                ss.str(),
                std::vector<model::CarrierRouting::my_bool>(output_channels, 0)});
    }

    //  by default, we'll just send the first channel to all outputs
    proc::fill(ret.front().channel, 1);
    return ret;
}

std::vector<model::ImpulseRouting> init_hardware_channels(
        const AudioDeviceManager& m) {
    const auto active_channels =
            m.getCurrentAudioDevice()->getActiveOutputChannels();
    const auto names = m.getCurrentAudioDevice()->getOutputChannelNames();
    std::vector<model::ImpulseRouting> ret;
    for (auto i = 0u; i != names.size(); ++i) {
        if (active_channels[i]) {
            ret.emplace_back(model::ImpulseRouting{names[i].toStdString()});
        }
    }
    return ret;
}

}  // namespace

class ConvolutionRoutingComponent::Impl : public Component,
                                          public ChangeBroadcaster,
                                          public model::BroadcastListener,
                                          public DragAndDropContainer {
public:
    Impl(AudioDeviceManager& audio_device_manager, int carrier_channels)
            : carrier_data(init_carrier_channels(
                      carrier_channels,
                      audio_device_manager.getCurrentAudioDevice()
                              ->getActiveOutputChannels()
                              .countNumberOfSetBits()))
            , impulse_data(init_hardware_channels(audio_device_manager)) {
        addAndMakeVisible(carrier_panel);
        addAndMakeVisible(impulse_panel);

        carrier_connector.trigger();
    }

    void receive_broadcast(model::Broadcaster* b) override {
        connectors.clear();

        //  for each component in the carrier list
        for (auto i = 0u; i != carrier_panel.getNumRows(); ++i) {
            //  get a handle to the component
            if (auto comp = dynamic_cast<CarrierRoutingComponent*>(
                        carrier_panel.getComponentForRowNumber(i))) {
                //  for each channel in the carrier
                const auto& channel = comp->routing.channel;
                for (auto j = 0u; j != channel.size(); ++j) {
                    //  if the channel is linked
                    if (channel[j]) {
                        connectors.push_back(std::make_unique<Connector>(
                                carrier_panel, impulse_panel, i, j));
                    }
                }
            }
        }

        for (const auto& i : connectors) {
            addAndMakeVisible(*i);
        }

        resized();

        sendChangeMessage();
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(padding);

        auto panel_width = 200;
        carrier_panel.setBounds(
                bounds.removeFromLeft(panel_width)
                        .withHeight(carrier_panel.get_desired_height()));
        impulse_panel.setBounds(
                bounds.removeFromRight(panel_width)
                        .withHeight(impulse_panel.get_desired_height()));

        for (const auto& i : connectors) {
            i->setBounds(getLocalBounds());
        }
    }

    int get_desired_height() const {
        return std::max(carrier_panel.get_desired_height(),
                        impulse_panel.get_desired_height()) +
               2 * padding;
    }

    const std::vector<model::CarrierRouting>& get_carrier_routing() const {
        return carrier_data;
    }
    const std::vector<model::ImpulseRouting>& get_impulse_routing() const {
        return impulse_data;
    }

    void push_carrier_buffer(const float** input,
                             int num_channels,
                             int num_samples) {
        carrier_panel.push_buffer(input, num_channels, num_samples);
    }

    void push_impulse_buffer(const float** input,
                             int num_channels,
                             int num_samples) {
        impulse_panel.push_buffer(input, num_channels, num_samples);
    }

    static const int padding{20};

private:
    std::vector<model::CarrierRouting> carrier_data;
    std::vector<model::ImpulseRouting> impulse_data;

    model::ValueWrapper<std::vector<model::CarrierRouting>> carrier_model{
            nullptr, carrier_data};
    model::ValueWrapper<std::vector<model::ImpulseRouting>> impulse_model{
            nullptr, impulse_data};

    model::BroadcastConnector carrier_connector{&carrier_model, this};

    CarrierRoutingPanel carrier_panel{"carrier", carrier_model, 30};
    ImpulseRoutingPanel impulse_panel{"hardware", impulse_model, 60};

    class Connector : public Component {
    public:
        Connector(CarrierRoutingPanel& carrier_panel,
                  ImpulseRoutingPanel& hardware_panel,
                  int from,
                  int to)
                : carrier_panel(carrier_panel)
                , hardware_panel(hardware_panel)
                , from(from)
                , to(to) {
        }

        void paint(Graphics& g) override {
            g.setColour(isMouseOver() ? VisualiserLookAndFeel::emphasis
                                      : Colours::lightgrey);
            g.strokePath(path, PathStrokeType(4));
        }

        void resized() override {
            path.clear();

            auto begin = carrier_panel.getComponentForRowNumber(from);
            auto begin_area = getLocalArea(begin, begin->getLocalBounds());
            auto begin_point = begin_area.getCentre()
                                       .withX(begin_area.getRight())
                                       .toFloat();
            auto end = hardware_panel.getComponentForRowNumber(to);
            auto end_area = getLocalArea(end, end->getLocalBounds());
            auto end_point =
                    end_area.getCentre().withX(end_area.getX()).toFloat();

            path.startNewSubPath(begin_point.toFloat());
            auto mid_point = (begin_point.getX() + end_point.getX()) * 0.5;
            path.cubicTo(Point<float>(mid_point, begin_point.getY()),
                         Point<float>(mid_point, end_point.getY()),
                         end_point);
        }

        bool hitTest(int x, int y) override {
            Point<float> ret;
            path.getNearestPoint(Point<float>(x, y), ret);
            return ret.getDistanceFrom(Point<float>(x, y)) < 4;
        }

        void mouseEnter(const MouseEvent& e) override {
            //  draw with new colours
            repaint();
        }

        void mouseExit(const MouseEvent& e) override {
            //  draw with new colours
            repaint();
        }

        void mouseDown(const MouseEvent& e) override {
            //  if rmb
            if (!e.mods.isPopupMenu()) {
                //  remove the connector by editing the model
                assert(0 <= from);
                assert(0 <= to);

                auto& model = carrier_panel.get_model();
                assert(from < model.size());
                auto& channel = model[from].channel;
                assert(to < channel.size());
                channel[to].set(0);
            }
        }

    private:
        CarrierRoutingPanel& carrier_panel;
        ImpulseRoutingPanel& hardware_panel;

        int from, to;

        Path path;
    };

    std::vector<std::unique_ptr<Connector>> connectors;
};

//----------------------------------------------------------------------------//

ConvolutionRoutingComponent::ConvolutionRoutingComponent(
        AudioDeviceManager& audio_device_manager, int carrier_channels)
        : audio_device_manager(audio_device_manager)
        , carrier_channels(carrier_channels)
        , pimpl(std::make_unique<Impl>(audio_device_manager,
                                       carrier_channels)) {
    pimpl->addChangeListener(this);
    addAndMakeVisible(*pimpl);
}
ConvolutionRoutingComponent::~ConvolutionRoutingComponent() noexcept = default;

void ConvolutionRoutingComponent::resized() {
    pimpl->setBounds(getLocalBounds());
}

void ConvolutionRoutingComponent::changeListenerCallback(
        ChangeBroadcaster* cb) {
    if (cb == &audio_device_manager) {
        pimpl = std::make_unique<Impl>(audio_device_manager, carrier_channels);
        pimpl->addChangeListener(this);
        addAndMakeVisible(*pimpl);
        resized();
    } else if (cb == pimpl.get()) {
        sendChangeMessage();
    }
}

int ConvolutionRoutingComponent::get_desired_height() const {
    return pimpl->get_desired_height();
}

void ConvolutionRoutingComponent::carrier_signal_in(ConvolutionAudioSource*,
                                                    const float** input,
                                                    int num_channels,
                                                    int num_samples) {
    pimpl->push_carrier_buffer(input, num_channels, num_samples);
}
void ConvolutionRoutingComponent::impulse_signal_in(ConvolutionAudioSource*,
                                                    const float** input,
                                                    int num_channels,
                                                    int num_samples) {
    pimpl->push_impulse_buffer(input, num_channels, num_samples);
}

const std::vector<model::CarrierRouting>&
ConvolutionRoutingComponent::get_carrier_routing() const {
    return pimpl->get_carrier_routing();
}
const std::vector<model::ImpulseRouting>&
ConvolutionRoutingComponent::get_impulse_routing() const {
    return pimpl->get_impulse_routing();
}