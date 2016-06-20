#pragma once

#include "FullModel.hpp"

template <typename T>
class TextDisplayProperty : public PropertyComponent,
                            public model::BroadcastListener {
public:
    TextDisplayProperty(const String& name,
                        int height,
                        model::ValueWrapper<T>& value)
            : PropertyComponent(name, height)
            , value(value) {
        addAndMakeVisible(label);
        value_connector.trigger();
    }

    void refresh() override {
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &value) {
            std::stringstream ss;
            ss << value;
            label.setText(ss.str(), dontSendNotification);
        }
    }

private:
    model::ValueWrapper<T>& value;
    model::BroadcastConnector value_connector{&value, this};

    Label label;
};