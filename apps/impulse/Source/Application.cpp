#include "Application.hpp"
#include "CommandIDs.h"
#include "LoadWindow.hpp"

ImpulseApplication::MainWindow::MainWindow(String name, const File& file)
        : DocumentWindow(name, Colours::darkgrey.darker(), allButtons)
        , this_file(file)
        , content_component(file) {
    content_component.setSize(800, 500);
    setContentNonOwned(&content_component, true);
    setUsingNativeTitleBar(true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    setResizable(true, false);
    setResizeLimits(400, 300, 100000, 100000);
    setVisible(true);

    auto& command_manager = ImpulseApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    addKeyListener(command_manager.getKeyMappings());
}

void ImpulseApplication::MainWindow::closeButtonPressed() {
    auto& main_windows = ImpulseApplication::get_app().main_windows;
    auto it = std::find_if(main_windows.begin(),
                           main_windows.end(),
                           [this](const auto& i) { return i.get() == this; });
    if (it != main_windows.end()) {
        main_windows.erase(it);
    }

    ImpulseApplication::get_app().show_hide_load_window();
}

void ImpulseApplication::MainWindow::getAllCommands(
        Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idCloseProject});
}
void ImpulseApplication::MainWindow::getCommandInfo(
        CommandID command_id, ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idCloseProject:
            result.setInfo("Close", "Close the current project", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress('w', ModifierKeys::commandModifier, 0));
            break;
        default:
            break;
    }
}
bool ImpulseApplication::MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idCloseProject:
            closeButtonPressed();
            return true;

        default:
            return false;
    }
}
ApplicationCommandTarget*
ImpulseApplication::MainWindow::getNextCommandTarget() {
    return &get_app();
}

//----------------------------------------------------------------------------//

namespace {
StoredSettings& get_app_settings() {
    return *ImpulseApplication::get_app().stored_settings;
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
    AsyncQuitRetrier() {
        startTimer(500);
    }

    void timerCallback() override {
        stopTimer();
        delete this;

        if (auto* app = JUCEApplicationBase::getInstance()) {
            app->systemRequestedQuit();
        }
    }

    JUCE_DECLARE_NON_COPYABLE(AsyncQuitRetrier)
};

const String ImpulseApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String ImpulseApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool ImpulseApplication::moreThanOneInstanceAllowed() {
    return false;
}

void ImpulseApplication::initialise(const String& commandLine) {
    LookAndFeel::setDefaultLookAndFeel(&look_and_feel);

    PropertiesFile::Options options;
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
    options.folderName = "~/.config/impulse";
#else
    options.folderName = "impulse";
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

void ImpulseApplication::shutdown() {
    main_windows.clear();

    MenuBarModel::setMacMainMenu(nullptr);

    main_menu_bar_model = nullptr;
    command_manager = nullptr;

    stored_settings = nullptr;
}

void ImpulseApplication::systemRequestedQuit() {
    if (ModalComponentManager::getInstance()->cancelAllModalComponents()) {
        new ::AsyncQuitRetrier();
    } else {
        attempt_close_all();
        if (main_windows.empty()) {
            quit();
        }
    }
}

void ImpulseApplication::attempt_close_all() {
    while (!main_windows.empty()) {
        auto size = main_windows.size();
        (*main_windows.begin())->closeButtonPressed();
        if (main_windows.size() >= size) {
            //  failed to remove the window
            return;
        }
    }
}

void ImpulseApplication::anotherInstanceStarted(const String& commandLine) {
}

void ImpulseApplication::file_dropped(FileDropComponent*, const File& f) {
    open_project(f);
}

void ImpulseApplication::show_hide_load_window() {
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

//----------------------------------------------------------------------------//

ImpulseApplication& ImpulseApplication::get_app() {
    auto i = dynamic_cast<ImpulseApplication*>(JUCEApplication::getInstance());
    jassert(i != nullptr);
    return *i;
}

ApplicationCommandManager& ImpulseApplication::get_command_manager() {
    auto i = ImpulseApplication::get_app().command_manager.get();
    jassert(i);
    return *i;
}

void ImpulseApplication::create_file_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idOpenProject);

    PopupMenu recent;
    stored_settings->recent_files.createPopupMenuItems(
            recent, recent_projects_base_id, true, true);
    menu.addSubMenu("Open Recent", recent);

    menu.addSeparator();

    menu.addCommandItem(&get_command_manager(), CommandIDs::idCloseProject);
#if !JUCE_MAC
    menu.addSeparator();
    menu.addCommandItem(&get_command_manager(),
                        StandardApplicationCommandIDs::quit);
#endif
}

void ImpulseApplication::create_view_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(),
                        CommandIDs::idShowAudioPreferences);
}

void ImpulseApplication::handle_main_menu_command(int menu_item_id) {
    if (menu_item_id >= recent_projects_base_id) {
        open_project(stored_settings->recent_files.getFile(
                menu_item_id - recent_projects_base_id));
    }
}

void ImpulseApplication::getAllCommands(Array<CommandID>& commands) {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({
            CommandIDs::idOpenProject,
    });
}
void ImpulseApplication::getCommandInfo(CommandID command_id,
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
        default:
            JUCEApplication::getCommandInfo(command_id, result);
            break;
    }
}

bool ImpulseApplication::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idOpenProject:
            open_project_from_dialog();
            return true;
        default:
            return JUCEApplication::perform(info);
    }
    return true;
}

void ImpulseApplication::open_project(const File& file) {
    try {
        main_windows.insert(
                std::make_unique<MainWindow>(getApplicationName(), file));
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

std::string ImpulseApplication::get_valid_file_formats() {
    return "*.wav,*.aiff,*.aif";
}

void ImpulseApplication::open_project_from_dialog() {
    FileChooser fc("open project", File::nonexistent, get_valid_file_formats());
    if (fc.browseForFileToOpen()) {
        open_project(fc.getResult());
    }
}

ImpulseApplication::MainMenuBarModel::MainMenuBarModel() {
    setApplicationCommandManagerToWatch(&get_command_manager());
}

StringArray ImpulseApplication::MainMenuBarModel::getMenuBarNames() {
    return {"File", "View"};
}

PopupMenu ImpulseApplication::MainMenuBarModel::getMenuForIndex(
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

void ImpulseApplication::MainMenuBarModel::menuItemSelected(
        int menu_item_id, int top_level_menu_index) {
    get_app().handle_main_menu_command(menu_item_id);
}

START_JUCE_APPLICATION(ImpulseApplication)