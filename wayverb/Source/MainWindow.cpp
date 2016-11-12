#include "MainWindow.h"
#include "Application.h"
#include "CommandIDs.h"

//  init from as much outside info as possible
MainWindow::MainWindow(String name, std::string fname)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons)
        , model_{std::move(fname)}
        , content_component_{model_} {
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
    // delete help_window_;
    removeKeyListener(
            WayverbApplication::get_command_manager().getKeyMappings());
}

void MainWindow::closeButtonPressed() {
    if (model_.needs_save()) {
        switch (NativeMessageBox::showYesNoCancelBox(
                AlertWindow::AlertIconType::WarningIcon,
                "save?",
                "There are unsaved changes. Do you wish to save?")) {
            case 0:  // cancel
                return;

            case 1:  // yes
                //  Attempt to save. Show a dialog if something goes wrong.
                try_and_explain([&] { save(); }, "saving project");

                //  If the model still needs saving for some reason (the user
                //  cancelled, an error ocurred), just return now.
                if (model_.needs_save()) {
                    return;
                }

                //  Everything's fine, so carry on.
                break;

            case 2:  // no
                break;
        }
    }

    auto& main_windows = WayverbApplication::get_app().main_windows;
    auto it = std::find_if(begin(main_windows),
                           end(main_windows),
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
                       CommandIDs::idVisualise});
}

void MainWindow::getCommandInfo(CommandID command_id,
                                ApplicationCommandInfo& result) {
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
            //  TODO
            //  result.setTicked(wrapper.render_state.visualise.get());
            //  result.setActive(!wrapper.render_state.is_rendering.get());
            break;

        default: break;
    }
}
bool MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idSaveProject:
            try_and_explain([&] { save(); }, "saving");
            return true;

        case CommandIDs::idSaveAsProject:
            try_and_explain([&] { save_as(); }, "saving as");
            return true;

        case CommandIDs::idCloseProject: closeButtonPressed(); return true;

        case CommandIDs::idVisualise:
            //  TODO
            //  wrapper.render_state.visualise.set(
            //         !wrapper.render_state.visualise.get());
            return true;

        default: return false;
    }
}

ApplicationCommandTarget* MainWindow::getNextCommandTarget() {
    return &get_app();
}

void MainWindow::save() {
    model_.save([this] { return browse_for_file_to_save(); });
}

void MainWindow::save_as() {
    if (const auto fname = browse_for_file_to_save()) {
        model_.save_as(*fname);
    }
}

std::experimental::optional<std::string> browse_for_file_to_save() {
    FileChooser fc{"save location...", File(), "*.way"};
    return fc.browseForFileToSave(true)
                   ? std::experimental::make_optional(
                             fc.getResult().getFullPathName().toStdString())
                   : std::experimental::nullopt;
}

/*
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
*/
