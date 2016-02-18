#pragma once

#include "MainComponent.hpp"

class VisualiserApplication final : public JUCEApplication {
public:
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String& commandLine) override;

    class MainWindow final : public DocumentWindow {
    public:
        MainWindow(String name);
        void closeButtonPressed() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};