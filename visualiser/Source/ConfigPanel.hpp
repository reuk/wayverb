#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class FileLoaderButton final : public Component, public Button::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() noexcept = default;
        virtual void fileLoaded(FileLoaderButton * flb, const File & f) = 0;
    };

    FileLoaderButton(const std::string & buttonText,
                     const std::string & validPatterns);

    void resized() override;

    void buttonClicked(Button * button) override;

    void addListener(Listener & l);
    void removeListener(Listener & l);

    File getFile() const;

private:
    std::unique_ptr<TextButton> textButton;
    std::unique_ptr<Label> label;
    ListenerList<Listener> listenerList;
    std::string validPatterns;
    File file;
};

class ConfigPanel final : public Component,
                          public FileLoaderButton::Listener,
                          public Button::Listener {
public:
    ConfigPanel();

    void resized() override;

    void fileLoaded(FileLoaderButton * flb, const File & f) override;
    void buttonClicked(Button * b) override;

    void updateLaunchButton();

private:
    std::unique_ptr<FileLoaderButton> objectButton;
    std::unique_ptr<FileLoaderButton> materialButton;
    std::unique_ptr<FileLoaderButton> configButton;
    std::unique_ptr<TextButton> launchButton;
};