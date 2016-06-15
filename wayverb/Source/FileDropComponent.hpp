#pragma once

#include "connector.h"

class FileDropComponent : public Component,
                          public FileDragAndDropTarget,
                          public Button::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void file_dropped(FileDropComponent*, const File& f) = 0;
    };

    FileDropComponent(const std::string& back, const std::string& button);

    void paint(Graphics& g) override;
    void resized() override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void fileDragEnter(const StringArray& files, int x, int y) override;
    void fileDragExit(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    void buttonClicked(Button* b) override;

    void set_valid_file_formats(const std::string& f);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    bool file_drag{false};
    std::string back_text;

    std::string valid_file_formats{"*"};

    TextButton load_button;
    model::Connector<TextButton> load_connector{&load_button, this};

    ListenerList<Listener> listener_list;
};