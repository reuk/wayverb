#pragma once

#include "MainComponent.hpp"
#include "VisualiserLookAndFeel.hpp"

class VisualiserApplication final : public JUCEApplication {
public:
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;

    void initialise(const String& commandLine) override;
    void shutdown() override;

    void systemRequestedQuit() override;
    void anotherInstanceStarted(const String& commandLine) override;

    static VisualiserApplication& get_app();
    static ApplicationCommandManager& get_command_manager();

    static const auto recent_projects_base_id = 100;

    void create_file_menu(PopupMenu& menu);

    void handle_main_menu_command(int menu_item_id);

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    void open_project(const File& file);
    void open_project_from_dialog();

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
        MainWindow(String name, const File& project);
        ~MainWindow() noexcept;

        void closeButtonPressed() override;

        void save_as_project();

        void getAllCommands(Array<CommandID>& commands) override;
        void getCommandInfo(CommandID command_id,
                            ApplicationCommandInfo& result) override;
        bool perform(const InvocationInfo& info) override;
        ApplicationCommandTarget* getNextCommandTarget() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    VisualiserLookAndFeel look_and_feel;
    SharedResourcePointer<TooltipWindow> tooltip_window;

    std::unique_ptr<ApplicationCommandManager> command_manager;
    std::unique_ptr<MainMenuBarModel> main_menu_bar_model;

    std::unique_ptr<MainWindow> main_window;

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