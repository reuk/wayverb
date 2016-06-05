#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class FileDropComponent : public Component, public FileDragAndDropTarget {
public:
    FileDropComponent();

    void paint(Graphics& g) override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void fileDragEnter(const StringArray& files, int x, int y) override;
    void fileDragExit(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    void mouseDown(const MouseEvent& e) override;

private:
    bool file_drag{false};
};