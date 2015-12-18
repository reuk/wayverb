#include "MainComponent.h"

MainContentComponent::MainContentComponent()
        : modelRendererComponent(std::make_unique<ModelRendererComponent>()) {
    addAndMakeVisible(modelRendererComponent.get());
    setSize(600, 400);
}

void MainContentComponent::paint(Graphics& g) {
}

void MainContentComponent::resized() {
    modelRendererComponent->setBounds(getLocalBounds());
}
