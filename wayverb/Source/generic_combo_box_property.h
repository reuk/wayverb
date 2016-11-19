#pragma once

#include "generic_property_component.h"

template <typename Model, typename Enum>
class generic_combo_box_property
        : public generic_property_component<Model, Enum, ComboBox> {
public:
    using get_text = std::function<std::string(Enum)>;

    generic_combo_box_property(

            Model& model,
            const String& name,
            const std::initializer_list<Enum>& enum_values,
            get_text get_text)
            : generic_property_component<Model, Enum, ComboBox>{model, name, 25}
            , get_text_{std::move(get_text)} {
        for (auto i : enum_values) {
            this->content.addItem(get_text_(i), static_cast<int>(i));
        }
    }

    void comboBoxChanged(ComboBox* cb) override {
        this->controller_updated(static_cast<Enum>(cb->getSelectedId()));
    }

protected:
    ~generic_combo_box_property() noexcept = default;

private:
    void set_view(const Enum& e) override {
        this->content.setSelectedId(static_cast<int>(e), dontSendNotification);
    }

    get_text get_text_;
};
