#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class FileDropComponent : public juce::Component,
                          public juce::FileDragAndDropTarget,
                          public juce::Button::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void file_dropped(FileDropComponent*, const juce::File& f) = 0;
    };

    FileDropComponent(const std::string& back, const std::string& button);
    virtual ~FileDropComponent() noexcept;

    void paint(juce::Graphics& g) override;
    void resized() override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void buttonClicked(juce::Button* b) override;

    void set_valid_file_formats(const std::string& f);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    bool file_drag{false};
    std::string back_text;

    std::string valid_file_formats{"*"};

    juce::TextButton load_button;

    juce::ListenerList<Listener> listener_list;
};
