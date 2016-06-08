#include "FileDropComponent.hpp"

#include "CommandIDs.h"
#include "Main.hpp"

FileDropComponent::FileDropComponent() {
    setSize(600, 400);
}

void FileDropComponent::paint(Graphics& g) {
    g.fillAll(Colours::black);

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
        g.setColour(Colours::darkgrey);
    } else {
        g.setColour(Colour(0x30, 0x30, 0x30));
    }
    g.fillPath(d);
    g.setFont(40);
    g.setColour(Colours::darkgrey);
    g.drawFittedText("drop a project file here, or\nclick to browse",
                     0,
                     0,
                     getWidth(),
                     getHeight(),
                     Justification::centred,
                     2);
}

bool FileDropComponent::isInterestedInFileDrag(const StringArray& files) {
    WildcardFileFilter filter(VisualiserApplication::get_valid_file_formats(),
                              "*",
                              "valid extensions");
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
    VisualiserApplication::get_app().open_project(files[0]);
}

void FileDropComponent::mouseDown(const MouseEvent& e) {
    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.invokeDirectly(CommandIDs::idOpenProject, true);
}
