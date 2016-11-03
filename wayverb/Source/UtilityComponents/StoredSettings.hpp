#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct TargetOS {
    enum OS { windows = 0, osx, linux, unknown };

    static OS get_this_os() noexcept;
};

class StoredSettings : public juce::ValueTree::Listener {
public:
    StoredSettings(const std::string& app_name,
                   const juce::PropertiesFile::Options& options);
    virtual ~StoredSettings();

    juce::PropertiesFile& get_global_properties();
    juce::PropertiesFile& get_project_properties(const std::string& uid);

    void flush();
    void reload();

    juce::RecentlyOpenedFilesList recent_files;

    juce::Array<juce::File> get_last_projects();
    void set_last_projects(const juce::Array<juce::File>& files);

private:
    std::string app_name;
    juce::PropertiesFile::Options options;

    juce::OwnedArray<juce::PropertiesFile> property_files;
    juce::ValueTree project_defaults;

    juce::PropertiesFile* create_props_file(const std::string& fname);

    void changed();

    void update_global_preferences();
    void update_recent_files();

    void valueTreePropertyChanged(juce::ValueTree&,
                                  const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&,
                               juce::ValueTree&,
                               int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreeParentChanged(juce::ValueTree&) override;
};
