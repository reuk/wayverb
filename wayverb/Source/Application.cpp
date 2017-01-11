#include "Application.h"
#include "AngularLookAndFeel.h"
#include "CommandIDs.h"
#include "try_and_explain.h"

#include "UtilityComponents/LoadWindow.h"

#include "core/serialize/surface.h"

#include <fstream>
#include <memory>

namespace {

auto get_options() {
    PropertiesFile::Options options;
    options.filenameSuffix = "settings";
    options.osxLibrarySubFolder = "Application Support";
#if JUCE_LINUX
    options.folderName = "~/.config/wayverb";
#else
    options.folderName = "wayverb";
#endif
    return options;
}

class AutoDeleteDocumentWindow : public DocumentWindow {
public:
    using DocumentWindow::DocumentWindow;
    void closeButtonPressed() override { delete this; }
};
}  // namespace

class wayverb_application::instance final : public ApplicationCommandTarget,
                                            public FileDropComponent::Listener {
public:
    //  Setup/Teardown.

    instance(wayverb_application& owner, const std::string& /*command_line*/)
            : owner_{owner}
            , stored_settings_{owner.getApplicationName().toStdString(),
                               get_options()}
            , main_menu_bar_model_{command_manager_, stored_settings_} {

        main_menu_bar_model_.connect_recent_file_selected(
                [&](auto fname) { open_project(fname); });

        LookAndFeel::setDefaultLookAndFeel(&look_and_feel_);

        command_manager_.registerAllCommandsForTarget(this);
        command_manager_.getKeyMappings()->resetToDefaultMappings();

        MenuBarModel::setMacMainMenu(&main_menu_bar_model_, nullptr);
        main_menu_bar_model_.menuItemsChanged();

        show_hide_load_window();
    }

    ~instance() noexcept {
        MenuBarModel::setMacMainMenu(nullptr);
    }

    //  Commands.

    ApplicationCommandTarget* getNextCommandTarget() override {
        return &owner_;
    }

    void getAllCommands(Array<CommandID>& commands) override {
        commands.addArray({
                CommandIDs::idOpenProject,
        });
    }

    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override {
        switch (command_id) {
            case CommandIDs::idOpenProject: {
                result.setInfo("Open Project...",
                               "Open an existing project",
                               "General",
                               0);
                result.defaultKeypresses.add(
                        KeyPress('o', ModifierKeys::commandModifier, 0));
                break;
            }
        }
    }

    bool perform(const InvocationInfo& info) override {
        switch (info.commandID) {
            case CommandIDs::idOpenProject: {
                open_project_from_dialog();
                return true;
            }
        }

        return false;
    }

    //  File dropped.

    void file_dropped(FileDropComponent*, const File& f) override {
        open_project(f.getFullPathName().toStdString());
    }

    //  Before quitting.

    void attempt_close_all() {
        //  IMPORTANT
        //  You can't just loop through windows and call closeButtonPressed on
        //  each, because the windows will be removed from the map while it is
        //  being iterated, invalidating the iterators.
        //  Instead, we manually check if each is ready to be deleted, and
        //  erase it if so.
        for (auto it = cbegin(main_windows_); it != cend(main_windows_);) {
            it = (*it)->prepare_to_close() ? main_windows_.erase(it) : ++it;
        }
    }

    bool ready_to_quit() const { return main_windows_.empty(); }

    StoredSettings& get_app_settings() { return stored_settings_; }
    const StoredSettings& get_app_settings() const { return stored_settings_; }

    ApplicationCommandManager& get_command_manager() {
        return command_manager_;
    }
    const ApplicationCommandManager& get_command_manager() const {
        return command_manager_;
    }

    LookAndFeel& get_look_and_feel() { return look_and_feel_; }

private:
    class main_menu_bar_model : public MenuBarModel {
    public:
        main_menu_bar_model(ApplicationCommandManager& command_manager,
                            StoredSettings& stored_settings)
                : command_manager_{command_manager}
                , stored_settings_{stored_settings} {
            setApplicationCommandManagerToWatch(&command_manager_);
        }

        StringArray getMenuBarNames() override { return {"File", "View"}; }

        PopupMenu getMenuForIndex(int /*top_level_menu_index*/,
                                  const String& menu_name) override {
            PopupMenu menu;
            if (menu_name == "File") {
                create_file_menu(command_manager_, menu);
            } else if (menu_name == "View") {
                create_view_menu(command_manager_, menu);
            } else {
                jassertfalse;
            }
            return menu;
        }

        void menuItemSelected(int menu_item_id,
                              int /*top_level_menu_index*/) override {
            if (menu_item_id >= recent_projects_base_id) {
                recent_file_selected_(
                        stored_settings_.recent_files
                                .getFile(menu_item_id - recent_projects_base_id)
                                .getFullPathName()
                                .toStdString());
            }
        }

        void create_file_menu(ApplicationCommandManager& command_manager,
                              PopupMenu& menu) {
            menu.addCommandItem(&command_manager, CommandIDs::idOpenProject);

            PopupMenu recent;
            stored_settings_.recent_files.createPopupMenuItems(
                    recent, recent_projects_base_id, true, true);
            menu.addSubMenu("Open Recent", recent);

            menu.addSeparator();

            menu.addCommandItem(&command_manager, CommandIDs::idCloseProject);
            menu.addCommandItem(&command_manager, CommandIDs::idSaveProject);
            menu.addCommandItem(&command_manager, CommandIDs::idSaveAsProject);

            menu.addSeparator();

            menu.addCommandItem(&command_manager, CommandIDs::idStartRender);
            menu.addCommandItem(&command_manager, CommandIDs::idCancelRender);

#if !JUCE_MAC
            menu.addSeparator();
            menu.addCommandItem(&command_manager,
                                StandardApplicationCommandIDs::quit);
#endif
        }

        void create_view_menu(ApplicationCommandManager& command_manager,
                              PopupMenu& menu) {
            menu.addCommandItem(&command_manager, CommandIDs::idVisualise);
            menu.addCommandItem(&command_manager, CommandIDs::idResetView);
        }

        using recent_file_selected = util::event<std::string>;
        recent_file_selected::connection connect_recent_file_selected(
                recent_file_selected::callback_type callback) {
            return recent_file_selected_.connect(std::move(callback));
        }

    private:
        static constexpr auto recent_projects_base_id = 100;

        ApplicationCommandManager& command_manager_;
        StoredSettings& stored_settings_;

        util::event<std::string> recent_file_selected_;
    };

    void open_project(const std::string& fname) {
        try_and_explain(
                [&] {
                    auto new_window = std::make_unique<main_window>(
                            *this, owner_.getApplicationName(), fname);

                    //  When window asks to close, find it in the set and delete
                    //  it.
                    new_window->connect_wants_to_close([this](auto& window) {
                        //  Look up the window in the list of open windows.
                        const auto it =
                                std::find_if(cbegin(main_windows_),
                                             cend(main_windows_),
                                             [&](const auto& ptr) {
                                                 return ptr.get() == &window;
                                             });

                        main_windows_.erase(it);

                        show_hide_load_window();
                    });

                    main_windows_.insert(std::move(new_window));
                    register_recent_file(fname);
                    show_hide_load_window();
                },
                "opening project",
                "Make sure the file is a 3D object or wayverb project.");
    }

    void open_project_from_dialog() {
        FileChooser fc("open project", File::nonexistent, valid_file_formats);
        if (fc.browseForFileToOpen()) {
            open_project(fc.getResult().getFullPathName().toStdString());
        }
    }

    void show_hide_load_window() {
        //  Show load window only if there are no other open windows.
        //  This instance should handle commands if there are no main windows
        //  open, otherwise allow the front window to handle commands.
        if (ready_to_quit()) {
            load_window_ = [this] {
                auto ret = std::make_unique<LoadWindow>(
                        owner_.getApplicationName(),
                        DocumentWindow::closeButton,
                        valid_file_formats,
                        get_command_manager());
                ret->addListener(this);
                return ret;
            }();
            command_manager_.setFirstCommandTarget(this);
        } else {
            load_window_ = nullptr;
            command_manager_.setFirstCommandTarget(nullptr);
        }
    }

    static constexpr const char* valid_file_formats =
            "*.way;*.fbx;*.dae;*.gltf;*.glb;*.blend;*.3ds;*.ase;*.obj;*.ifc;*."
            "xgl;*.zgl;*.ply;*.dxf;*.lwo;*.lws;*.lxo;*.stl;*.x;*.ac;*.ms3d;*."
            "cob;*.scn";

    class wide_property_component_look_and_feel final
            : public AngularLookAndFeel {
    public:
        //  Don't bother drawing anything.
        void drawPropertyComponentBackground(Graphics&,
                                             int,
                                             int,
                                             PropertyComponent&) override {}
        void drawPropertyComponentLabel(Graphics&,
                                        int,
                                        int,
                                        PropertyComponent&) override {}

        //  Let the content take up the entire space.
        Rectangle<int> getPropertyComponentContentPosition(
                PropertyComponent& c) override {
            return c.getLocalBounds();
        }
    };

    wayverb_application& owner_;

    AngularLookAndFeel look_and_feel_;
    wide_property_component_look_and_feel
            wide_property_component_look_and_feel_;

    StoredSettings stored_settings_;

    ApplicationCommandManager command_manager_;
    main_menu_bar_model main_menu_bar_model_;

    std::unique_ptr<DocumentWindow> load_window_;

    std::unordered_set<std::unique_ptr<main_window>> main_windows_;

    SharedResourcePointer<TooltipWindow> tooltip_window_;
};

constexpr const char* wayverb_application::instance::valid_file_formats;

////////////////////////////////////////////////////////////////////////////////

StoredSettings& wayverb_application::get_app_settings() {
    return get_app().instance_->get_app_settings();
}

PropertiesFile& wayverb_application::get_global_properties() {
    return get_app_settings().get_global_properties();
}

void wayverb_application::register_recent_file(const std::string& file) {
    RecentlyOpenedFilesList::registerRecentFileNatively(File(file));
    get_app_settings().recent_files.addFile(File(file));
    get_app_settings().flush();
}

//  taken from the Projucer, no h8 plx
struct async_quit_retrier final : private Timer {
    async_quit_retrier() { startTimer(500); }

    async_quit_retrier(const async_quit_retrier&) = delete;
    async_quit_retrier(async_quit_retrier&&) noexcept = delete;

    async_quit_retrier& operator=(const async_quit_retrier&) = delete;
    async_quit_retrier& operator=(async_quit_retrier&&) noexcept = delete;

    void timerCallback() override {
        stopTimer();
        delete this;

        if (auto* app = JUCEApplicationBase::getInstance()) {
            app->systemRequestedQuit();
        }
    }
};

const String wayverb_application::getApplicationName() {
    return ProjectInfo::projectName;
}

const String wayverb_application::getApplicationVersion() {
    return ProjectInfo::versionString;
}

bool wayverb_application::moreThanOneInstanceAllowed() { return false; }

void wayverb_application::initialise(const String& command_line) {
    instance_ = std::make_unique<instance>(*this, command_line.toStdString());
}

void wayverb_application::shutdown() { instance_ = nullptr; }

void wayverb_application::systemRequestedQuit() {
    if (ModalComponentManager::getInstance()->cancelAllModalComponents()) {
        new async_quit_retrier();
    } else {
        instance_->attempt_close_all();
        if (instance_->ready_to_quit()) {
            quit();
        }
    }
}

void wayverb_application::anotherInstanceStarted(
        const String& /*command_line*/) {}

wayverb_application& wayverb_application::get_app() {
    auto i = dynamic_cast<wayverb_application*>(JUCEApplication::getInstance());
    jassert(i != nullptr);
    return *i;
}

LookAndFeel& wayverb_application::get_look_and_feel() {
    jassert(get_app().instance_);
    return get_app().instance_->get_look_and_feel();
}

ApplicationCommandManager& wayverb_application::get_command_manager() {
    jassert(get_app().instance_);
    return get_app().instance_->get_command_manager();
}

START_JUCE_APPLICATION(wayverb_application)
