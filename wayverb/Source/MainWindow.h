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

    void show_help();

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

private:
    wayverb::combined::model::app model_;
    MainContentComponent content_component_;
};
