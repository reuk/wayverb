#include "Main.hpp"
#include "CommandIDs.h"

#include "Presets.hpp"

#include "combined/config_serialize.h"
#include "common/surface_serialize.h"

#include <memory>

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

const String VisualiserApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String VisualiserApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool VisualiserApplication::moreThanOneInstanceAllowed() {
    return false;
}

void VisualiserApplication::initialise(const String& commandLine) {
    LookAndFeel::setDefaultLookAndFeel(&look_and_feel);

    command_manager = std::make_unique<ApplicationCommandManager>();
    command_manager->registerAllCommandsForTarget(this);

    main_menu_bar_model = std::make_unique<MainMenuBarModel>();

    MenuBarModel::setMacMainMenu(main_menu_bar_model.get(), nullptr);

    //    command_manager->invoke(CommandIDs::idOpenProject, false);
}

void VisualiserApplication::shutdown() {
    main_window = nullptr;

    MenuBarModel::setMacMainMenu(nullptr);

    main_menu_bar_model = nullptr;
    command_manager = nullptr;
}

void VisualiserApplication::systemRequestedQuit() {
    if (ModalComponentManager::getInstance()->cancelAllModalComponents()) {
        new ::AsyncQuitRetrier();
    } else {
        quit();
    }
}

void VisualiserApplication::anotherInstanceStarted(const String& commandLine) {
}

//  init from as much outside info as possible
VisualiserApplication::MainWindow::MainWindow(String name,
                                              SceneData&& scene_data,
                                              model::FullModel&& model)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons)
        , scene_data(std::move(scene_data))
        , model(std::move(model))
        , content_component(this->scene_data, wrapper) {
    setUsingNativeTitleBar(true);
    setContentNonOwned(&content_component, true);

    centreWithSize(getWidth(), getHeight());
    setVisible(true);

    setResizable(true, false);

    setVisible(true);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    command_manager.getKeyMappings()->resetToDefaultMappings();
    addKeyListener(command_manager.getKeyMappings());
    setWantsKeyboardFocus(false);
}

static model::FullModel construct_full_model(
    const model::Persistent& persistent) {
    return model::FullModel{persistent,
                            model::get_presets(),
                            model::FullReceiverConfig{},
                            model::RenderState{}};
}

static File get_sub_path(const File& way, const std::string& name) {
    return way.getChildFile(name.c_str());
}

File VisualiserApplication::MainWindow::get_model_path(const File& way) {
    return get_sub_path(way, "model.model");
}

File VisualiserApplication::MainWindow::get_config_path(const File& way) {
    return get_sub_path(way, "config.json");
}

std::tuple<SceneData, model::FullModel>
VisualiserApplication::MainWindow::scene_and_model_from_file(const File& f) {
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
        return std::make_tuple(std::move(scene_data),
                               construct_full_model(config));
    };

    auto is_not_way = [&f] {
        //  try to load the model
        SceneData scene_data(f.getFullPathName().toStdString());
        //  return the pair
        return std::make_tuple(
            std::move(scene_data),
            construct_full_model(model::Persistent{
                config::Combined{}, scene_data.get_materials()}));
    };

    return f.getFileExtension() == ".way" ? is_way() : is_not_way();
}

//  given just a file, work out whether it's a project or a 3d model, then init
//  appropriately
VisualiserApplication::MainWindow::MainWindow(String name, const File& f)
        : MainWindow(name, scene_and_model_from_file(f)) {
}

VisualiserApplication::MainWindow::MainWindow(
    String name, std::tuple<SceneData, model::FullModel>&& p)
        : MainWindow(
              name, std::move(std::get<0>(p)), std::move(std::get<1>(p))) {
}

VisualiserApplication::MainWindow::~MainWindow() noexcept {
    //  TODO if you need to save, do it here (?)

    removeKeyListener(
        VisualiserApplication::get_command_manager().getKeyMappings());
}

void VisualiserApplication::MainWindow::receive_broadcast(
    model::Broadcaster* b) {
    if (b == &wrapper.render_state.is_rendering) {
        VisualiserApplication::get_command_manager().commandStatusChanged();
    } else if (b == &wrapper.render_state.visualise) {
        VisualiserApplication::get_command_manager().commandStatusChanged();
    }
}

void VisualiserApplication::MainWindow::getAllCommands(
    Array<CommandID>& commands) {
    commands.addArray({
        CommandIDs::idSaveAsProject,
        CommandIDs::idCloseProject,
        CommandIDs::idVisualise,
    });
}
void VisualiserApplication::MainWindow::getCommandInfo(
    CommandID command_id, ApplicationCommandInfo& result) {
    switch (command_id) {
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
            result.setTicked(wrapper.render_state.visualise);
            result.setActive(!wrapper.render_state.is_rendering);
            break;
        default:
            break;
    }
}
bool VisualiserApplication::MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idSaveAsProject:
            save_as_project();
            return true;

        case CommandIDs::idCloseProject:
            closeButtonPressed();
            return true;

        case CommandIDs::idVisualise:
            wrapper.render_state.visualise.toggle();
            return true;

        default:
            return false;
    }
}

ApplicationCommandTarget*
VisualiserApplication::MainWindow::getNextCommandTarget() {
    return &get_app();
}

void VisualiserApplication::MainWindow::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}

void VisualiserApplication::MainWindow::save_as_project() {
    FileChooser fc("save project as", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        auto root = fc.getResult();
        root.createDirectory();

        //  write current geometry to file
        scene_data.save(get_model_path(root).getFullPathName().toStdString());

        //  write config with all current materials to file
        std::ofstream stream(
            get_config_path(root).getFullPathName().toStdString());
        cereal::JSONOutputArchive archive(stream);
        archive(cereal::make_nvp("persistent", model.persistent));
    }
}

VisualiserApplication& VisualiserApplication::get_app() {
    auto i =
        dynamic_cast<VisualiserApplication*>(JUCEApplication::getInstance());
    jassert(i != nullptr);
    return *i;
}

ApplicationCommandManager& VisualiserApplication::get_command_manager() {
    auto i = VisualiserApplication::get_app().command_manager.get();
    jassert(i);
    return *i;
}

void VisualiserApplication::create_file_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idOpenProject);

    menu.addSeparator();

    menu.addCommandItem(&get_command_manager(), CommandIDs::idCloseProject);
    menu.addCommandItem(&get_command_manager(), CommandIDs::idSaveAsProject);

#if !JUCE_MAC
    menu.addSeparator();
    menu.addCommandItem(&get_command_manager(),
                        StandardApplicationCommandIDs::quit);
#endif
}

void VisualiserApplication::create_view_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idVisualise);
}

void VisualiserApplication::handle_main_menu_command(int menu_item_id) {
    if (menu_item_id >= recent_projects_base_id) {
        //  TODO open recent project
    }
}

void VisualiserApplication::getAllCommands(Array<CommandID>& commands) {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({
        CommandIDs::idOpenProject,
    });
}
void VisualiserApplication::getCommandInfo(CommandID command_id,
                                           ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idOpenProject:
            result.setInfo(
                "Open Project...", "Open an existing project", "General", 0);
            result.defaultKeypresses.add(
                KeyPress('o', ModifierKeys::commandModifier, 0));
            break;
        default:
            JUCEApplication::getCommandInfo(command_id, result);
            break;
    }
}

bool VisualiserApplication::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idOpenProject:
            open_project_from_dialog();
            return true;
        default:
            return JUCEApplication::perform(info);
    }
    return true;
}

void VisualiserApplication::open_project(const File& file) {
    main_window = std::make_unique<MainWindow>(getApplicationName(), file);
}

void VisualiserApplication::open_project_from_dialog() {
    FileChooser fc("open project", File::nonexistent, "*.way,*.obj,*.model");
    if (fc.browseForFileToOpen()) {
        open_project(fc.getResult());
    }
}

VisualiserApplication::MainMenuBarModel::MainMenuBarModel() {
    setApplicationCommandManagerToWatch(&get_command_manager());
}

StringArray VisualiserApplication::MainMenuBarModel::getMenuBarNames() {
    return {"File", "View"};
}

PopupMenu VisualiserApplication::MainMenuBarModel::getMenuForIndex(
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

void VisualiserApplication::MainMenuBarModel::menuItemSelected(
    int menu_item_id, int top_level_menu_index) {
    get_app().handle_main_menu_command(menu_item_id);
}

START_JUCE_APPLICATION(VisualiserApplication)
