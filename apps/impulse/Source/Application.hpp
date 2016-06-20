#pragma once

#include "ImpulseViewerTabs.hpp"

#include "AngularLookAndFeel.hpp"
#include "FileDropComponent.hpp"
#include "StoredSettings.hpp"

#include <unordered_set>

class ImpulseApplication final : public JUCEApplication,
                                 public FileDropComponent::Listener {
public:
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String& commandLine) override;

    static ImpulseApplication& get_app();
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

    class MainWindow : public DocumentWindow, public ApplicationCommandTarget {
    public:
        MainWindow(String name, const File& file);
        virtual ~MainWindow() noexcept = default;

        void closeButtonPressed() override;

        void getAllCommands(Array<CommandID>& commands) override;
        void getCommandInfo(CommandID command_id,
                            ApplicationCommandInfo& result) override;
        bool perform(const InvocationInfo& info) override;
        ApplicationCommandTarget* getNextCommandTarget() override;

    private:
        File this_file;
        ImpulseViewerTabs content_component;
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
