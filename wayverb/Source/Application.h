#pragma once

#include "main_window.h"

#include "UtilityComponents/FileDropComponent.h"
#include "UtilityComponents/StoredSettings.h"

#include <unordered_set>

class wayverb_application final : public JUCEApplication {
public:
    void initialise(const String& commandLine) override;
    void shutdown() override;

    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String& commandLine) override;

    static wayverb_application& get_app();
    static ApplicationCommandManager& get_command_manager();
    static StoredSettings& get_app_settings();
    static PropertiesFile& get_global_properties();
    static void register_recent_file(const std::string& file);

private:
    class instance;
    std::unique_ptr<instance> instance_;
};
