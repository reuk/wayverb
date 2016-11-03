#include "StoredSettings.hpp"

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

StoredSettings::StoredSettings(const std::string& app_name,
                               const juce::PropertiesFile::Options& options)
        : app_name(app_name)
        , options(options)
        , project_defaults("PROJECT_DEFAULT_SETTINGS") {
    reload();
    project_defaults.addListener(this);
}

StoredSettings::~StoredSettings() {
    project_defaults.removeListener(this);
    flush();
}

juce::PropertiesFile& StoredSettings::get_global_properties() {
    return *property_files.getUnchecked(0);
}

juce::PropertiesFile* StoredSettings::create_props_file(
        const std::string& fname) {
    auto these_options = options;
    these_options.applicationName = fname;
    return new juce::PropertiesFile(these_options);
}

juce::PropertiesFile& StoredSettings::get_project_properties(
        const std::string& uid) {
    auto fname = app_name + "_" + uid;
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
    property_files.add(create_props_file(app_name));

    juce::ScopedPointer<juce::XmlElement> projectDefaultsXml(
            property_files.getFirst()->getXmlValue("PROJECT_DEFAULT_SETTINGS"));

    if (projectDefaultsXml != nullptr)
        project_defaults = juce::ValueTree::fromXml(*projectDefaultsXml);

    recent_files.restoreFromString(
            get_global_properties().getValue("recentFiles"));
    recent_files.removeNonExistentFiles();
}

juce::Array<juce::File> StoredSettings::get_last_projects() {
    juce::StringArray s;
    s.addTokens(get_global_properties().getValue("lastProjects"), "|", "");

    juce::Array<juce::File> f;
    for (auto i = 0; i != s.size(); ++i)
        f.add(juce::File(s[i]));
    return f;
}

void StoredSettings::set_last_projects(const juce::Array<juce::File>& files) {
    juce::StringArray s;
    for (auto i = 0; i != files.size(); ++i) {
        s.add(files.getReference(i).getFullPathName());
    }

    get_global_properties().setValue("lastProjects", s.joinIntoString("|"));
}

void StoredSettings::changed() {
    juce::ScopedPointer<juce::XmlElement> data(project_defaults.createXml());
    property_files.getUnchecked(0)->setValue("PROJECT_DEFAULT_SETTINGS", data);
}

void StoredSettings::valueTreePropertyChanged(juce::ValueTree&,
                                              const juce::Identifier&) {
    changed();
}
void StoredSettings::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) {
    changed();
}
void StoredSettings::valueTreeChildRemoved(juce::ValueTree&,
                                           juce::ValueTree&,
                                           int) {
    changed();
}
void StoredSettings::valueTreeChildOrderChanged(juce::ValueTree&, int, int) {
    changed();
}
void StoredSettings::valueTreeParentChanged(juce::ValueTree&) {
    changed();
}
