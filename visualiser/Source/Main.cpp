#include "Main.hpp"
#include "glog/logging.h"

#include <memory>

const String VisualiserApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String VisualiserApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool VisualiserApplication::moreThanOneInstanceAllowed() {
    return true;
}

void VisualiserApplication::initialise(const String& commandLine) {
    google::InitGoogleLogging("visualiser");
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
}

void VisualiserApplication::shutdown() {
    mainWindow = nullptr;  // (deletes our window)
}

void VisualiserApplication::systemRequestedQuit() {
    quit();
}

void VisualiserApplication::anotherInstanceStarted(const String& commandLine) {
}

VisualiserApplication::MainWindow::MainWindow(String name)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    setContentOwned(new MainContentComponent(), true);

    centreWithSize(getWidth(), getHeight());
    setVisible(true);

    setResizable(true, false);
}

void VisualiserApplication::MainWindow::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}

START_JUCE_APPLICATION(VisualiserApplication)
