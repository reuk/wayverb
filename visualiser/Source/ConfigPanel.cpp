#include "ConfigPanel.hpp"

FileLoaderButton::FileLoaderButton(const std::string &buttonText,
                                   const std::string &validPatterns)
        : textButton(std::make_unique<TextButton>(buttonText))
        , label(std::make_unique<Label>("", ""))
        , validPatterns(validPatterns) {
    addAndMakeVisible(textButton.get());
    addAndMakeVisible(label.get());

    textButton->addListener(this);
}

void FileLoaderButton::resized() {
    auto padding = 4;
    textButton->setBounds(0, 0, getWidth(), (getHeight() - padding) / 2);
    label->setBounds(0,
                     (getHeight() + padding) / 2,
                     getWidth(),
                     (getHeight() - padding) / 2);
}

void FileLoaderButton::buttonClicked(Button *button) {
    if (button == textButton.get()) {
        FileChooser fc("open...", File::nonexistent, validPatterns, true);
        if (fc.browseForFileToOpen()) {
            file = fc.getResult();
            label->setText(file.getFileName(), dontSendNotification);
            listenerList.call(&Listener::fileLoaded, this, fc.getResult());
        }
    }
}

void FileLoaderButton::addListener(Listener &l) {
    listenerList.add(&l);
}

void FileLoaderButton::removeListener(Listener &l) {
    listenerList.remove(&l);
}

File FileLoaderButton::getFile() const {
    return file;
}

ConfigPanel::ConfigPanel()
        : objectButton(
              std::make_unique<FileLoaderButton>("load object", "*.obj"))
        , materialButton(
              std::make_unique<FileLoaderButton>("load material", "*.json"))
        , configButton(
              std::make_unique<FileLoaderButton>("load config", "*.json"))
        , launchButton(std::make_unique<TextButton>("launch")) {
    addAndMakeVisible(objectButton.get());
    addAndMakeVisible(materialButton.get());
    addAndMakeVisible(configButton.get());
    addAndMakeVisible(launchButton.get());

    objectButton->addListener(*this);
    materialButton->addListener(*this);
    configButton->addListener(*this);
    launchButton->addListener(this);

    updateLaunchButton();
    refreshView();
}

void ConfigPanel::resized() {
    auto padding = 4;
    objectButton->setSize((getWidth() - padding * 3) / 4, getHeight());
    materialButton->setSize((getWidth() - padding * 3) / 4, getHeight());
    configButton->setSize((getWidth() - padding * 3) / 4, getHeight());
    launchButton->setSize((getWidth() - padding * 3) / 4, getHeight());

    objectButton->setTopLeftPosition(0, 0);
    materialButton->setTopLeftPosition(objectButton->getRight() + padding, 0);
    configButton->setTopLeftPosition(materialButton->getRight() + padding, 0);
    launchButton->setTopLeftPosition(configButton->getRight() + padding, 0);
}

void ConfigPanel::fileLoaded(FileLoaderButton *flb, const File &f) {
    updateLaunchButton();
    refreshView();
}

void ConfigPanel::buttonClicked(Button *b) {
    if (b == launchButton.get()) {
    }
}

void ConfigPanel::updateLaunchButton() {
    launchButton->setEnabled(objectButton->getFile().exists() &&
                             materialButton->getFile().exists() &&
                             configButton->getFile().exists());
}

void ConfigPanel::addListener(ConfigPanel::Listener &l) {
    listenerList.add(&l);
}

void ConfigPanel::removeListener(ConfigPanel::Listener &l) {
    listenerList.remove(&l);
}

void ConfigPanel::refreshView() {
    listenerList.call(&Listener::filesChanged,
                      this,
                      objectButton->getFile(),
                      materialButton->getFile(),
                      configButton->getFile());
}