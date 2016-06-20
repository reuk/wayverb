#include "FileDropComponent.hpp"
#include "AngularLookAndFeel.hpp"

FileDropComponent::FileDropComponent(const std::string& back,
                                     const std::string& button)
        : back_text(back)
        , load_button(button) {
    addAndMakeVisible(load_button);
}

void FileDropComponent::addListener(Listener* l) {
    listener_list.add(l);
}
void FileDropComponent::removeListener(Listener* l) {
    listener_list.remove(l);
}

void FileDropComponent::buttonClicked(Button* b) {
    if (b == &load_button) {
        FileChooser fc("open...", File::nonexistent, valid_file_formats);
        if (fc.browseForFileToOpen()) {
            listener_list.call(&Listener::file_dropped, this, fc.getResult());
        }
    }
}

void FileDropComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);

    auto indent = 30;
    Path p;
    p.addRoundedRectangle(indent,
                          indent,
                          getWidth() - indent * 2,
                          getHeight() - indent * 2,
                          30);
    Path d;
    PathStrokeType pathStrokeType(8);
    float dashLengths[] = {8, 8};
    pathStrokeType.createDashedStroke(d, p, dashLengths, 2);

    if (file_drag) {
        g.setColour(AngularLookAndFeel::emphasis);
    } else {
        g.setColour(Colours::lightgrey);
    }
    g.fillPath(d);
    g.setFont(20);
    g.setColour(Colours::lightgrey);

    auto button_bounds = load_button.getBounds();
    auto text_bounds = button_bounds.withPosition(button_bounds.getX(),
                                                  button_bounds.getY() - 50);
    g.drawFittedText(back_text, text_bounds, Justification::centred, 2);
}

void FileDropComponent::resized() {
    load_button.centreWithSize(300, 50);
}

void FileDropComponent::set_valid_file_formats(const std::string& f) {
    valid_file_formats = f;
}

bool FileDropComponent::isInterestedInFileDrag(const StringArray& files) {
    WildcardFileFilter filter(valid_file_formats, "*", "valid extensions");
    return files.size() == 1 && filter.isFileSuitable(files[0]);
}
void FileDropComponent::fileDragEnter(const StringArray& files, int x, int y) {
    file_drag = true;
    repaint();
}
void FileDropComponent::fileDragExit(const StringArray& files) {
    file_drag = false;
    repaint();
}
void FileDropComponent::filesDropped(const StringArray& files, int x, int y) {
    assert(isInterestedInFileDrag(files));
    file_drag = false;
    repaint();
    listener_list.call(&Listener::file_dropped, this, files[0]);
}