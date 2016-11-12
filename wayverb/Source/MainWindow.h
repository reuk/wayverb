#pragma once

#include "MainContentComponent.h"

#include "combined/model/app.h"

class MainWindow final : public DocumentWindow,
                         public ApplicationCommandTarget {
public:
    //  load with a custom config too
    MainWindow(String name, std::string fname);
    ~MainWindow() noexcept;

    void closeButtonPressed() override;

    // void show_help();

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void save();
    void save_as();

    template <typename Callback>
    void try_and_explain(const Callback& callback,
                         const std::string& action_name) {
        try {
            callback();
        } catch (const std::exception& e) {
            AlertWindow::showMessageBoxAsync(
                    AlertWindow::AlertIconType::WarningIcon,
                    "warning",
                    util::build_string("Encountered an error ",
                                       action_name,
                                       ":\n",
                                       e.what()));
        }
    }

    //  TODO closeButtonPressed should notify the app that the window needs to
    //  close. It should *not* take matters into its own hands.

private:
    std::experimental::optional<std::string> browse_for_file_to_save();

    wayverb::combined::model::app model_;
    MainContentComponent content_component_;
};
