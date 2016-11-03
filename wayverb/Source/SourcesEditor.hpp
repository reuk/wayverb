#pragma once

#include "BasicPanel.hpp"
#include "HelpWindow.hpp"
#include "ValueWrapperEditableListBox.hpp"

#include "core/geo/box.h"

/*
class SourcesListBox : public ValueWrapperListBox<glm::vec3> {
public:
    using ValueWrapperListBox<glm::vec3>::ValueWrapperListBox;

private:
    std::unique_ptr<Component> new_component_for_row(int row,
                                                     bool selected) override;
};

//----------------------------------------------------------------------------//

using SourcesEditableListBox = ValueWrapperEditableListBox<SourcesListBox>;

//----------------------------------------------------------------------------//

class SourcesEditorPanel : public ListEditorPanel<SourcesEditableListBox> {
public:
    SourcesEditorPanel(model_type& model, const wayverb::core::geo::box& aabb);

private:
    std::unique_ptr<Component> new_editor(
            model::ValueWrapper<value_type>& v) override;

    wayverb::core::geo::box aabb;
};

//----------------------------------------------------------------------------//

class ReceiversListBox : public ValueWrapperListBox<model::ReceiverSettings> {
public:
    using ValueWrapperListBox<model::ReceiverSettings>::ValueWrapperListBox;

private:
    std::unique_ptr<Component> new_component_for_row(int row,
                                                     bool selected) override;
};

//----------------------------------------------------------------------------//

using ReceiversEditableListBox = ValueWrapperEditableListBox<ReceiversListBox>;

//----------------------------------------------------------------------------//

class ReceiversEditorPanel : public ListEditorPanel<ReceiversEditableListBox> {
public:
    ReceiversEditorPanel(model_type& model, const wayverb::core::geo::box& aabb);

private:
    std::unique_ptr<Component> new_editor(model::ValueWrapper<value_type>& v) override;

    wayverb::core::geo::box aabb;
};
*/
