#include "MainComponent.h"

MainContentComponent::MainContentComponent()
        : modelRendererComponent(std::make_unique<ModelRendererComponent>())
//        , configPanel(std::make_unique<ConfigPanel>())
{
    addAndMakeVisible(modelRendererComponent.get());
    //    addAndMakeVisible(configPanel.get());
    setSize(600, 400);

    //    configPanel->addListener(*modelRendererComponent);
}

void MainContentComponent::paint(Graphics& g) {
}

void MainContentComponent::resized() {
    //    auto panelHeight = 50;
    //    modelRendererComponent->setBounds(
    //        getLocalBounds().removeFromTop(getHeight() - panelHeight));
    //    configPanel->setBounds(getLocalBounds().removeFromBottom(panelHeight));

    modelRendererComponent->setBounds(getLocalBounds());
}
