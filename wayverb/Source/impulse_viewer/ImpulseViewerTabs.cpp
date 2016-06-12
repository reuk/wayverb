#include "ImpulseViewerTabs.hpp"

#include "ConvolutionViewer.hpp"
#include "ImpulseViewer.hpp"
#include "Main.hpp"

ImpulseViewerTabs::ImpulseViewerTabs(const File& impulse_file)
        : TabbedComponent(TabbedButtonBar::TabsAtTop) {
    addTab("ir inspector",
           Colours::darkgrey,
           new ImpulseViewer(
                   audio_device_manager, audio_format_manager, impulse_file),
           true);

    addTab("auralisation",
           Colours::darkgrey,
           new ConvolutionViewer(
                   audio_device_manager, audio_format_manager, impulse_file),
           true);

    setWantsKeyboardFocus(false);
    getTabContentComponent(0)->grabKeyboardFocus();

    setSize(800, 500);
}
