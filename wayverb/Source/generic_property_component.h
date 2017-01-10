#pragma once

#include "UtilityComponents/connector.h"

template <typename Content>
class property_component_adapter : public PropertyComponent {
public:
    using content_t = Content;

    template <typename... Ts>
    property_component_adapter(const String& name, int height, Ts&&... ts)
            : PropertyComponent{name, height}
            , content{std::forward<Ts>(ts)...} {
        addAndMakeVisible(content);
    }

    void refresh() override {}

    content_t content;
};

template <typename Model, typename Value, typename Content>
class generic_property_component : public property_component_adapter<Content>,
                                   public Content::Listener {
public:
    using model_t = Model;
    using value_t = Value;

    template <typename... Ts>
    generic_property_component(model_t& model,
                               const String& name,
                               int height,
                               Ts&&... ts)
            : property_component_adapter<Content>{name,
                                                  height,
                                                  std::forward<Ts>(ts)...}
            , model_{model} {}

    void controller_updated(const value_t& value) {
        this->set_model(model_, value);
    }

    void update_from_model() { this->set_view(this->get_model(this->model_)); }

protected:
    ~generic_property_component() noexcept = default;

private:
    virtual void set_model(model_t& model, const value_t& value) = 0;
    virtual value_t get_model(const model_t& model) const = 0;
    virtual void set_view(const value_t& value) = 0;

    model_t& model_;

    model::Connector<Content> content_connector_{&this->content, this};
    typename model_t::scoped_connection connection_{
            model_.connect([this](auto& model) { this->update_from_model(); })};
};


template <typename Model>
class text_display_property
        : public generic_property_component<Model, std::string, Label> {
public:
    text_display_property(Model& model, const String& name)
            : generic_property_component<Model, std::string, Label>{
                      model, name, 25} {}

private:
    void labelTextChanged(Label*) override {}
    void set_model(Model& model, const std::string& value) override {}
    void set_view(const std::string& value) override {
        this->content.setText(value, dontSendNotification);
    }
};
