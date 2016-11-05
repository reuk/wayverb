#pragma once

#include "MainContentComponent.hpp"

#include "model/model.h"

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
