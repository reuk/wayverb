#pragma once

#include "BasicPanel.hpp"
#include "HelpWindow.hpp"
#include "ValueWrapperEditableListBox.hpp"

#include "model/model.h"

#include "core/geo/box.h"

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

private:
    std::unique_ptr<Component> new_editor() ;

    wayverb::core::geo::box aabb;
};

//----------------------------------------------------------------------------//

class ReceiversListBox : public ValueWrapperListBox<model::receiver> {
public:
    using ValueWrapperListBox<model::receiver>::ValueWrapperListBox;

private:
    std::unique_ptr<Component> new_component_for_row(int row,
                                                     bool selected) override;
};

//----------------------------------------------------------------------------//

using ReceiversEditableListBox = ValueWrapperEditableListBox<ReceiversListBox>;

//----------------------------------------------------------------------------//

class ReceiversEditorPanel : public ListEditorPanel<ReceiversEditableListBox> {
public:
    ReceiversEditorPanel();

private:
    std::unique_ptr<Component> new_editor() ;
};
