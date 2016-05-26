#include "StoredSettings.hpp"

#include "Main.hpp"

TargetOS::OS TargetOS::get_this_os() noexcept {
#if JUCE_WINDOWS
    return windows;
#elif JUCE_MAC
    return osx;
#elif JUCE_LINUX
    return linux;
#else
    return unknown;
#endif
}

StoredSettings& get_app_settings() {
    return *VisualiserApplication::get_app().stored_settings;
}

PropertiesFile& get_global_properties() {
    return get_app_settings().get_global_properties();
}

StoredSettings::StoredSettings()
        : project_defaults("PROJECT_DEFAULT_SETTINGS") {
    reload();
    project_defaults.addListener(this);
}

StoredSettings::~StoredSettings() {
    project_defaults.removeListener(this);
    flush();
}

PropertiesFile& StoredSettings::get_global_properties() {
    return *property_files.getUnchecked(0);
}

static PropertiesFile* create_props_file(const std::string& fname) {
    return new PropertiesFile(
        VisualiserApplication::get_property_file_options_for(fname));
}

PropertiesFile& StoredSettings::get_project_properties(const std::string& uid) {
    auto fname = std::string("wayverb_") + uid;
    for (auto i = property_files.size(); --i >= 0;) {
        auto props = property_files.getUnchecked(i);
        if (props->getFile().getFileNameWithoutExtension() == fname)
            return *props;
    }

    auto p = create_props_file(fname);
    property_files.add(p);
    return *p;
}

void StoredSettings::update_global_preferences() {
    update_recent_files();
}

void StoredSettings::update_recent_files() {
    get_global_properties().setValue("recentFiles", recent_files.toString());
}

void StoredSettings::flush() {
    update_global_preferences();
    for (auto i = property_files.size(); --i >= 0;) {
        property_files.getUnchecked(i)->saveIfNeeded();
    }
}

void StoredSettings::reload() {
    property_files.clear();
    property_files.add(create_props_file("wayverb"));

    ScopedPointer<XmlElement> projectDefaultsXml(
        property_files.getFirst()->getXmlValue("PROJECT_DEFAULT_SETTINGS"));

    if (projectDefaultsXml != nullptr)
        project_defaults = ValueTree::fromXml(*projectDefaultsXml);

    recent_files.restoreFromString(
        get_global_properties().getValue("recentFiles"));
    recent_files.removeNonExistentFiles();
}

Array<File> StoredSettings::get_last_projects() {
    StringArray s;
    s.addTokens(get_global_properties().getValue("lastProjects"), "|", "");

    Array<File> f;
    for (auto i = 0; i != s.size(); ++i)
        f.add(File(s[i]));
    return f;
}

void StoredSettings::set_last_projects(const Array<File>& files) {
    StringArray s;
    for (auto i = 0; i != files.size(); ++i) {
        s.add(files.getReference(i).getFullPathName());
    }

    get_global_properties().setValue("lastProjects", s.joinIntoString("|"));
}

void StoredSettings::changed() {
    ScopedPointer<XmlElement> data(project_defaults.createXml());
    property_files.getUnchecked(0)->setValue("PROJECT_DEFAULT_SETTINGS", data);
}

void StoredSettings::valueTreePropertyChanged(ValueTree&, const Identifier&) {
    changed();
}
void StoredSettings::valueTreeChildAdded(ValueTree&, ValueTree&) {
    changed();
}
void StoredSettings::valueTreeChildRemoved(ValueTree&, ValueTree&, int) {
    changed();
}
void StoredSettings::valueTreeChildOrderChanged(ValueTree&, int, int) {
    changed();
}
void StoredSettings::valueTreeParentChanged(ValueTree&) {
    changed();
}
