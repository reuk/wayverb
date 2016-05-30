#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct TargetOS {
    enum OS { windows = 0, osx, linux, unknown };

    static OS get_this_os() noexcept;
};

class StoredSettings : public ValueTree::Listener {
public:
    StoredSettings();
    virtual ~StoredSettings();

    PropertiesFile& get_global_properties();
    PropertiesFile& get_project_properties(const std::string& uid);

    void flush();
    void reload();

    RecentlyOpenedFilesList recent_files;

    Array<File> get_last_projects();
    void set_last_projects(const Array<File>& files);

private:
    OwnedArray<PropertiesFile> property_files;
    ValueTree project_defaults;

    void changed();

    void update_global_preferences();
    void update_recent_files();

    void valueTreePropertyChanged(ValueTree&, const Identifier&) override;
    void valueTreeChildAdded(ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved(ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged(ValueTree&, int, int) override;
    void valueTreeParentChanged(ValueTree&) override;
};

StoredSettings& get_app_settings();
PropertiesFile& get_global_properties();
