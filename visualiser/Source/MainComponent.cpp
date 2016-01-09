#include "MainComponent.h"

MainContentComponent::MainContentComponent()
        : modelRendererComponent(std::make_unique<ModelRendererComponent>())
        , demo_panel(std::make_unique<DemoPanel>()) {
    addAndMakeVisible(modelRendererComponent.get());
    addAndMakeVisible(demo_panel.get());

    demo_panel->add_listener(*modelRendererComponent);

    setSize(600, 400);
}

void MainContentComponent::paint(Graphics& g) {
}

void MainContentComponent::resized() {
    auto panelHeight = 50;
    modelRendererComponent->setBounds(
        getLocalBounds().removeFromTop(getHeight() - panelHeight));
    demo_panel->setBounds(getLocalBounds().removeFromBottom(panelHeight));
}
