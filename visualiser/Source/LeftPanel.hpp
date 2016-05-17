#pragma once

#include "BottomPanel.hpp"
#include "FullModel.hpp"
#include "ModelWrapper.hpp"
#include "SurfaceModel.hpp"

class LeftPanel : public Component {
public:
    LeftPanel(model::ValueWrapper<model::FullModel>& model);

    void resized() override;

private:
    model::ValueWrapper<model::FullModel>& model;

    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
