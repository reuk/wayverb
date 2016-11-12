#include "Application.h"
#include "CommandIDs.h"
#include "HelpWindow.h"

#include "UtilityComponents/LoadWindow.h"

#include "core/serialize/surface.h"

#include <memory>
#include <fstream>

namespace {
StoredSettings& get_app_settings() {
    return *WayverbApplication::get_app().stored_settings;
}

PropertiesFile& get_global_properties() {
    return get_app_settings().get_global_properties();
}
}  // namespace

void register_recent_file(const std::string& file) {
    RecentlyOpenedFilesList::registerRecentFileNatively(File(file));
    get_app_settings().recent_files.addFile(File(file));
    get_app_settings().flush();
}

//  taken from the Projucer
//  no h8 plx
struct AsyncQuitRetrier : private Timer {
    AsyncQuitRetrier() { startTimer(500); }

    void timerCallback() override {
        stopTimer();
        delete this;

        if (auto* app = JUCEApplicationBase::getInstance()) {
            app->systemRequestedQuit();
        }
    }

    JUCE_DECLARE_NON_COPYABLE(AsyncQuitRetrier)
};

const String WayverbApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String WayverbApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool WayverbApplication::moreThanOneInstanceAllowed() { return false; }

void WayverbApplication::initialise(const String& commandLine) {
    LookAndFeel::setDefaultLookAndFeel(&look_and_feel);

    PropertiesFile::Options options;
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
    options.folderName = "~/.config/wayverb";
#else
    options.folderName = "wayverb";
#endif

    stored_settings = std::make_unique<StoredSettings>(
            getApplicationName().toStdString(), options);

    command_manager = std::make_unique<ApplicationCommandManager>();
    command_manager->registerAllCommandsForTarget(this);
    command_manager->getKeyMappings()->resetToDefaultMappings();

    main_menu_bar_model = std::make_unique<MainMenuBarModel>();

    MenuBarModel::setMacMainMenu(main_menu_bar_model.get(), nullptr);

    show_hide_load_window();
}

void WayverbApplication::shutdown() {
    main_windows.clear();

    MenuBarModel::setMacMainMenu(nullptr);

    main_menu_bar_model = nullptr;
    command_manager = nullptr;

    stored_settings = nullptr;
}

void WayverbApplication::systemRequestedQuit() {
    if (ModalComponentManager::getInstance()->cancelAllModalComponents()) {
        new ::AsyncQuitRetrier();
    } else {
        attempt_close_all();
        if (main_windows.empty()) {
            quit();
        }
    }
}

void WayverbApplication::attempt_close_all() {
    while (!main_windows.empty()) {
        auto size = main_windows.size();
        (*main_windows.begin())->closeButtonPressed();
        if (main_windows.size() >= size) {
            //  failed to remove the window
            return;
        }
    }
}

void WayverbApplication::anotherInstanceStarted(const String& commandLine) {}

void WayverbApplication::file_dropped(FileDropComponent*, const File& f) {
    open_project(f);
}

void WayverbApplication::show_hide_load_window() {
    auto constructed_load_window = [this] {
        auto ret = std::make_unique<LoadWindow>(getApplicationName(),
                                                DocumentWindow::closeButton,
                                                get_valid_file_formats(),
                                                get_command_manager());
        ret->addListener(this);
        return ret;
    };
    load_window = main_windows.empty() ? constructed_load_window() : nullptr;
}



WayverbApplication& WayverbApplication::get_app() {
    auto i = dynamic_cast<WayverbApplication*>(JUCEApplication::getInstance());
    jassert(i != nullptr);
    return *i;
}

ApplicationCommandManager& WayverbApplication::get_command_manager() {
    auto i = WayverbApplication::get_app().command_manager.get();
    jassert(i);
    return *i;
}

void WayverbApplication::create_file_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idOpenProject);

    PopupMenu recent;
    stored_settings->recent_files.createPopupMenuItems(
            recent, recent_projects_base_id, true, true);
    menu.addSubMenu("Open Recent", recent);

    menu.addSeparator();

    menu.addCommandItem(&get_command_manager(), CommandIDs::idCloseProject);
    menu.addCommandItem(&get_command_manager(), CommandIDs::idSaveProject);
    menu.addCommandItem(&get_command_manager(), CommandIDs::idSaveAsProject);

#if !JUCE_MAC
    menu.addSeparator();
    menu.addCommandItem(&get_command_manager(),
                        StandardApplicationCommandIDs::quit);
#endif
}

void WayverbApplication::create_view_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idVisualise);
    menu.addSeparator();
    menu.addCommandItem(&get_command_manager(), CommandIDs::idShowHelp);
    //    menu.addCommandItem(&get_command_manager(),
    //                        CommandIDs::idShowAudioPreferences);
}

void WayverbApplication::handle_main_menu_command(int menu_item_id) {
    if (menu_item_id >= recent_projects_base_id) {
        open_project(stored_settings->recent_files.getFile(
                menu_item_id - recent_projects_base_id));
    }
}

void WayverbApplication::getAllCommands(Array<CommandID>& commands) {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({
            CommandIDs::idOpenProject,
    });
}
void WayverbApplication::getCommandInfo(CommandID command_id,
                                        ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idOpenProject:
            result.setInfo("Open Project...",
                           "Open an existing project",
                           "General",
                           0);
            result.defaultKeypresses.add(
                    KeyPress('o', ModifierKeys::commandModifier, 0));
            break;
        default: JUCEApplication::getCommandInfo(command_id, result); break;
    }
}

bool WayverbApplication::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idOpenProject: open_project_from_dialog(); return true;
        default: return JUCEApplication::perform(info);
    }
    return true;
}

void WayverbApplication::open_project(const File& file) {
    try {
        main_windows.insert(
                std::make_unique<MainWindow>(getApplicationName(), project::load(file)));
        register_recent_file(file.getFullPathName().toStdString());
        show_hide_load_window();
    } catch (const std::exception& e) {
        NativeMessageBox::showMessageBox(
                AlertWindow::WarningIcon,
                "exception...",
                std::string("Encountered an exception: ") + e.what());
    } catch (...) {
        NativeMessageBox::showMessageBox(
                AlertWindow::WarningIcon,
                "exception...",
                std::string("Encountered an unknown exception."));
    }
}

std::string WayverbApplication::get_valid_file_formats() {
    return "*.way,*.obj,*.model";
}

void WayverbApplication::open_project_from_dialog() {
    FileChooser fc("open project", File::nonexistent, get_valid_file_formats());
    if (fc.browseForFileToOpen()) {
        open_project(fc.getResult());
    }
}

WayverbApplication::MainMenuBarModel::MainMenuBarModel() {
    setApplicationCommandManagerToWatch(&get_command_manager());
}

StringArray WayverbApplication::MainMenuBarModel::getMenuBarNames() {
    return {"File", "View"};
}

PopupMenu WayverbApplication::MainMenuBarModel::getMenuForIndex(
        int top_level_menu_index, const String& menu_name) {
    PopupMenu menu;
    if (menu_name == "File") {
        get_app().create_file_menu(menu);
    } else if (menu_name == "View") {
        get_app().create_view_menu(menu);
    } else {
        jassertfalse;
    }
    return menu;
}

void WayverbApplication::MainMenuBarModel::menuItemSelected(
        int menu_item_id, int top_level_menu_index) {
    get_app().handle_main_menu_command(menu_item_id);
}

START_JUCE_APPLICATION(WayverbApplication)
