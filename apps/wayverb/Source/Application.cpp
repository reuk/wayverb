#include "Application.hpp"
#include "CommandIDs.hpp"
#include "Presets.hpp"

#include "UtilityComponents/LoadWindow.hpp"

#include "combined/serialize/model.h"
#include "common/serialize/surface.h"

#include "HelpWindow.hpp"

#include <memory>

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

const String WayverbApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String WayverbApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool WayverbApplication::moreThanOneInstanceAllowed() {
    return false;
}

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

void WayverbApplication::anotherInstanceStarted(const String& commandLine) {
}

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

//----------------------------------------------------------------------------//

//  init from as much outside info as possible
WayverbApplication::MainWindow::MainWindow(String name,
                                           SceneData&& scene_data,
                                           model::FullModel&& model,
                                           File&& this_file)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons)
        , scene_data(std::move(scene_data))
        , wrapper(nullptr, model)
        , this_file(std::move(this_file))
        , content_component(this->scene_data, wrapper) {
    content_component.setSize(800, 500);
    setContentNonOwned(&content_component, true);
    setUsingNativeTitleBar(true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    setResizable(true, false);
    setResizeLimits(400, 300, 100000, 100000);
    setVisible(true);

    auto& command_manager = WayverbApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    addKeyListener(command_manager.getKeyMappings());

    wrapper.needs_save.set(false);
}

namespace {
model::FullModel construct_full_model(const model::Persistent& persistent) {
    return model::FullModel{
            persistent, model::get_presets(), model::RenderState{}};
}

File get_sub_path(const File& way, const std::string& name) {
    return way.getChildFile(name.c_str());
}
}  // namespace

File WayverbApplication::MainWindow::get_model_path(const File& way) {
    return get_sub_path(way, "model.model");
}

File WayverbApplication::MainWindow::get_config_path(const File& way) {
    return get_sub_path(way, "config.json");
}

std::tuple<SceneData, model::FullModel, File>
WayverbApplication::MainWindow::scene_and_model_from_file(const File& f) {
    auto is_way = [&f] {
        //  look inside for a model
        auto model_file = get_model_path(f);
        if (!model_file.existsAsFile()) {
            //  TODO show alert window
        }

        //  load the model
        SceneData scene_data(model_file.getFullPathName().toStdString());
        //  look inside for a config
        auto config_file = get_config_path(f);
        if (!config_file.existsAsFile()) {
            //  TODO show alert window
        }
        //  load the config
        std::ifstream stream(config_file.getFullPathName().toStdString());
        cereal::JSONInputArchive archive(stream);
        model::Persistent config;
        archive(cereal::make_nvp("persistent", config));

        //  make sure the surfaces from the config match the surfaces in the
        //  model - merge the config surfaces into the scene, and then
        //  replace the config surfaces with the scene ones
        scene_data.set_surfaces(config.materials);
        config.materials = scene_data.get_materials();

        //  return the pair
        return std::make_tuple(
                std::move(scene_data), construct_full_model(config), f);
    };

    auto is_not_way = [&f] {
        //  try to load the model
        SceneData scene_data(f.getFullPathName().toStdString());
        //  return the pair
        return std::make_tuple(
                std::move(scene_data),
                construct_full_model(model::Persistent{
                        model::App{}, scene_data.get_materials()}),
                File());
    };

    return f.getFileExtension() == ".way" ? is_way() : is_not_way();
}

//  given just a file, work out whether it's a project or a 3d model, then
//  init
//  appropriately
WayverbApplication::MainWindow::MainWindow(String name, const File& f)
        : MainWindow(name, scene_and_model_from_file(f)) {
}

WayverbApplication::MainWindow::MainWindow(
        String name, std::tuple<SceneData, model::FullModel, File>&& p)
        : MainWindow(name,
                     std::move(std::get<0>(p)),
                     std::move(std::get<1>(p)),
                     std::move(std::get<2>(p))) {
}

WayverbApplication::MainWindow::~MainWindow() noexcept {
    delete help_window;
    removeKeyListener(
            WayverbApplication::get_command_manager().getKeyMappings());
}

void WayverbApplication::MainWindow::closeButtonPressed() {
    if (needs_save()) {
        switch (NativeMessageBox::showYesNoCancelBox(
                AlertWindow::AlertIconType::WarningIcon,
                "save?",
                "There are unsaved changes. Do you wish to save?")) {
            case 0:  // cancel
                return;
            case 1:  // yes
                if (!save_project()) {
                    return;
                }
                break;
            case 2:  // no
                break;
        }
    }

    auto& main_windows = WayverbApplication::get_app().main_windows;
    auto it = proc::find_if(main_windows,
                            [this](const auto& i) { return i.get() == this; });
    if (it != main_windows.end()) {
        main_windows.erase(it);
    }

    WayverbApplication::get_app().show_hide_load_window();
}

void WayverbApplication::MainWindow::receive_broadcast(model::Broadcaster* b) {
    if (b == &wrapper.persistent) {
        wrapper.needs_save.set(true);
    } else if (b == &wrapper.render_state.is_rendering) {
        WayverbApplication::get_command_manager().commandStatusChanged();
    } else if (b == &wrapper.render_state.visualise) {
        WayverbApplication::get_command_manager().commandStatusChanged();
    }
}

void WayverbApplication::MainWindow::getAllCommands(
        Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idSaveProject,
                       CommandIDs::idSaveAsProject,
                       CommandIDs::idCloseProject,
                       CommandIDs::idVisualise,
                       CommandIDs::idShowHelp});
}
void WayverbApplication::MainWindow::getCommandInfo(
        CommandID command_id, ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idSaveProject:
            result.setInfo("Save...", "Save", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress('s', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::idSaveAsProject:
            result.setInfo("Save As...", "Save as", "General", 0);
            result.defaultKeypresses.add(KeyPress(
                    's',
                    ModifierKeys::commandModifier | ModifierKeys::shiftModifier,
                    0));
            break;

        case CommandIDs::idCloseProject:
            result.setInfo("Close", "Close the current project", "General", 0);
            result.defaultKeypresses.add(
                    KeyPress('w', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::idVisualise:
            result.setInfo("Visualise",
                           "Toggle display of ray and wave information",
                           "General",
                           0);
            result.setTicked(wrapper.render_state.visualise.get());
            result.setActive(!wrapper.render_state.is_rendering.get());
            break;

        case CommandIDs::idShowHelp:
            result.setInfo("Show Help Pane",
                           "Toggle display of help window",
                           "General",
                           0);
            break;

        default:
            break;
    }
}
bool WayverbApplication::MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idSaveProject:
            save_project();
            return true;

        case CommandIDs::idSaveAsProject:
            save_as_project();
            return true;

        case CommandIDs::idCloseProject:
            closeButtonPressed();
            return true;

        case CommandIDs::idVisualise:
            wrapper.render_state.visualise.set(
                    !wrapper.render_state.visualise.get());
            return true;

        case CommandIDs::idShowHelp:
            show_help();
            return true;

        default:
            return false;
    }
}

ApplicationCommandTarget*
WayverbApplication::MainWindow::getNextCommandTarget() {
    return &get_app();
}

namespace {
class AutoDeleteDocumentWindow : public DocumentWindow {
public:
    using DocumentWindow::DocumentWindow;
    void closeButtonPressed() override {
        delete this;
    }
};
}  // namespace

void WayverbApplication::MainWindow::show_help() {
    if (!help_window) {
        help_window = new AutoDeleteDocumentWindow(
                "help viewer", Colours::darkgrey, closeButton);
        auto panel = new HelpPanel;
        panel->setSize(200, 300);

        Rectangle<int> area(0, 0, 200, 300);
        RectanglePlacement placement(RectanglePlacement::xRight |
                                     RectanglePlacement::doNotResize);
        auto result = placement.appliedTo(area,
                                          Desktop::getInstance()
                                                  .getDisplays()
                                                  .getMainDisplay()
                                                  .userArea.reduced(20));

        help_window->setBounds(result);
        help_window->setContentOwned(panel, true);
        help_window->setResizable(false, false);
        help_window->setUsingNativeTitleBar(true);
        help_window->setVisible(true);
        help_window->setAlwaysOnTop(true);
    }
}

bool WayverbApplication::MainWindow::needs_save() const {
    return wrapper.needs_save.get();
}

bool WayverbApplication::MainWindow::save_project() {
    if (!this_file.exists()) {
        return save_as_project();
    } else if (needs_save()) {
        save_to(this_file);
        return true;
    }
    return false;
}

bool WayverbApplication::MainWindow::save_as_project() {
    FileChooser fc("save project as", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        auto root = fc.getResult();
        save_to(root);
        this_file = root;
        return true;
    }
    return false;
}

void WayverbApplication::MainWindow::save_to(const File& f) {
    f.createDirectory();

    //  write current geometry to file
    scene_data.save(get_model_path(f).getFullPathName().toStdString());

    //  write config with all current materials to file
    std::ofstream stream(get_config_path(f).getFullPathName().toStdString());
    cereal::JSONOutputArchive archive(stream);
    archive(cereal::make_nvp("persistent", wrapper.persistent.get()));

    register_recent_file(f.getFullPathName().toStdString());
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
        default:
            JUCEApplication::getCommandInfo(command_id, result);
            break;
    }
}

bool WayverbApplication::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idOpenProject:
            open_project_from_dialog();
            return true;
        default:
            return JUCEApplication::perform(info);
    }
    return true;
}

void WayverbApplication::open_project(const File& file) {
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
