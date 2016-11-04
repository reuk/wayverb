#pragma once

#include "MainComponent.hpp"
#include "AngularLookAndFeel.hpp"

#include "UtilityComponents/FileDropComponent.hpp"
#include "UtilityComponents/StoredSettings.hpp"

#include "core/scene_data_loader.h"

#include <unordered_set>

struct project final {
    struct data final {
        wayverb::core::scene_data_loader scene;
        model::Persistent data;
        
        static File get_model_path(const File& way);
        static File get_config_path(const File& way);

        static struct data load(const File& f);

        static void save_to(const struct data& pd, const File& f);
    };

    data data;
    File file;
    
    static project load(const File& file);
    
    static bool save_as(project& project);
    static bool save(project& project);
};

////////////////////////////////////////////////////////////////////////////////

class WayverbApplication final : public JUCEApplication,
                                 public FileDropComponent::Listener {
public:
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String& commandLine) override;

    static WayverbApplication& get_app();
    static ApplicationCommandManager& get_command_manager();
    static std::string get_valid_file_formats();
    static constexpr auto recent_projects_base_id = 100;

    void create_file_menu(PopupMenu& menu);
    void create_view_menu(PopupMenu& menu);

    void handle_main_menu_command(int menu_item_id);

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    void open_project(const File& file);
    void open_project_from_dialog();

    void show_hide_load_window();

    void attempt_close_all();

    void file_dropped(FileDropComponent*, const File& f) override;

    class MainMenuBarModel : public MenuBarModel {
    public:
        MainMenuBarModel();

        StringArray getMenuBarNames() override;
        PopupMenu getMenuForIndex(int top_level_menu_index,
                                  const String& menu_name) override;
        void menuItemSelected(int menu_item_id,
                              int top_level_menu_index) override;
    };

    class MainWindow final : public DocumentWindow,
                             public ApplicationCommandTarget {
    public:
        //  load with a custom config too
        MainWindow(String name, project project);
        virtual ~MainWindow() noexcept;

        void closeButtonPressed() override;

        bool needs_save() const;

        void show_help();

        void getAllCommands(Array<CommandID>& commands) override;
        void getCommandInfo(CommandID command_id,
                            ApplicationCommandInfo& result) override;
        bool perform(const InvocationInfo& info) override;
        ApplicationCommandTarget* getNextCommandTarget() override;

    private:
        project project_;
        MainContentComponent content_component_;
        Component::SafePointer<DocumentWindow> help_window_{nullptr};
        
        //model::ValueWrapper<model::FullModel> wrapper;

/*
        model::BroadcastConnector persistent_connector{&wrapper.persistent,
                                                       this};
        model::BroadcastConnector is_rendering_connector{
                &wrapper.render_state.is_rendering, this};
        model::BroadcastConnector visualising_connector{
                &wrapper.render_state.visualise, this};
*/
    };

    std::unique_ptr<StoredSettings> stored_settings;

private:
    AngularLookAndFeel look_and_feel;
    SharedResourcePointer<TooltipWindow> tooltip_window;

    std::unique_ptr<ApplicationCommandManager> command_manager;
    std::unique_ptr<MainMenuBarModel> main_menu_bar_model;

    std::unique_ptr<DocumentWindow> load_window;
    std::unordered_set<std::unique_ptr<MainWindow>> main_windows;

    class AsyncQuitRetrier : private Timer {
    public:
        AsyncQuitRetrier();
        virtual ~AsyncQuitRetrier() noexcept = default;

        AsyncQuitRetrier(const AsyncQuitRetrier&) = delete;
        AsyncQuitRetrier& operator=(const AsyncQuitRetrier&) = delete;
        AsyncQuitRetrier(AsyncQuitRetrier&&) = delete;
        AsyncQuitRetrier& operator=(AsyncQuitRetrier&&) = delete;

        void timerCallback() override;
    };
    std::unique_ptr<AsyncQuitRetrier> async_quit_retrier;
};
