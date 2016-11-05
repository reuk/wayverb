#include "MainWindow.hpp"

#include "Application.hpp"

//  init from as much outside info as possible
MainWindow::MainWindow(String name, project project)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons)
        , project_{std::move(project)} {
    content_component_.setSize(800, 500);
    setContentNonOwned(&content_component_, true);
    setUsingNativeTitleBar(true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    setResizable(true, false);
    setResizeLimits(400, 300, 100000, 100000);
    setVisible(true);

    auto& command_manager = WayverbApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    addKeyListener(command_manager.getKeyMappings());
}

MainWindow::~MainWindow() noexcept {
    delete help_window_;
    removeKeyListener(
            WayverbApplication::get_command_manager().getKeyMappings());
}

void MainWindow::closeButtonPressed() {
    if (needs_save()) {
        switch (NativeMessageBox::showYesNoCancelBox(
                AlertWindow::AlertIconType::WarningIcon,
                "save?",
                "There are unsaved changes. Do you wish to save?")) {
            case 0:  // cancel
                return;
            case 1:  // yes
                if (!project::save(project_)) {
                    return;
                }
                break;
            case 2:  // no
                break;
        }
    }

    auto& main_windows = WayverbApplication::get_app().main_windows;
    auto it = std::find_if(begin(main_windows), end(main_windows),
                            [this](const auto& i) { return i.get() == this; });
    if (it != main_windows.end()) {
        main_windows.erase(it);
    }

    WayverbApplication::get_app().show_hide_load_window();
}

void MainWindow::getAllCommands(Array<CommandID>& commands) {
    commands.addArray({CommandIDs::idSaveProject,
                       CommandIDs::idSaveAsProject,
                       CommandIDs::idCloseProject,
                       CommandIDs::idVisualise,
                       CommandIDs::idShowHelp});
}
void MainWindow::getCommandInfo(CommandID command_id, ApplicationCommandInfo& result) {
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
            //result.setTicked(wrapper.render_state.visualise.get());
            //result.setActive(!wrapper.render_state.is_rendering.get());
            break;

        case CommandIDs::idShowHelp:
            result.setInfo("Show Help Pane",
                           "Toggle display of help window",
                           "General",
                           0);
            break;

        default: break;
    }
}
bool MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idSaveProject: project::save(project_); return true;

        case CommandIDs::idSaveAsProject: project::save_as(project_); return true;

        case CommandIDs::idCloseProject: closeButtonPressed(); return true;

        case CommandIDs::idVisualise:
            //wrapper.render_state.visualise.set(
            //        !wrapper.render_state.visualise.get());
            return true;

        case CommandIDs::idShowHelp: show_help(); return true;

        default: return false;
    }
}

ApplicationCommandTarget* MainWindow::getNextCommandTarget() {
    return &get_app();
}

namespace {
class AutoDeleteDocumentWindow : public DocumentWindow {
public:
    using DocumentWindow::DocumentWindow;
    void closeButtonPressed() override { delete this; }
};
}  // namespace

void MainWindow::show_help() {
    if (!help_window_) {
        help_window_ = new AutoDeleteDocumentWindow(
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

        help_window_->setBounds(result);
        help_window_->setContentOwned(panel, true);
        help_window_->setResizable(false, false);
        help_window_->setUsingNativeTitleBar(true);
        help_window_->setVisible(true);
        help_window_->setAlwaysOnTop(true);
    }
}

bool MainWindow::needs_save() const {
    //  TODO
    // return wrapper.needs_save.get();
    return true;
}
