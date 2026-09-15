#include <JuceHeader.h>
#include "MainComponent.h"

class PulseApplication final : public juce::JUCEApplication
{
public:
    PulseApplication() = default;

    const juce::String getApplicationName() override { return "Pulse"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>();
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        MainWindow()
            : DocumentWindow("Pulse",
                             juce::Colours::black,
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);

            content = new MainComponent();

            // MainComponent decides *when* to go fullscreen (F11, Esc, the
            // Settings row); the window is what actually knows how.
            content->onToggleFullscreenRequested = [this] { setTrueFullScreen(! trueFullScreen); };
            content->onExitFullscreenRequested = [this]
            {
                if (trueFullScreen)
                    setTrueFullScreen(false);
            };

            setContentOwned(content, true);
            centreWithSize(1180, 720);
            setResizable(true, true);
            setVisible(true);

            content->grabKeyboardFocus();
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        // True borderless fullscreen: covers the ENTIRE monitor, including
        // the taskbar, with no title bar, no frame, and no visible edge.
        //
        // Two fixes from the previous attempt:
        //
        // 1. Switched from hand-assembling (setUsingNativeTitleBar(false) +
        //    manual totalArea bounds) to Desktop::setKioskModeComponent() -
        //    JUCE's own dedicated API for exactly this ("cover the whole
        //    screen, strip decorations"), rather than us re-deriving the
        //    same behaviour ourselves.
        //
        // 2. The actual cause of the thin left-edge sliver: TopLevelWindow
        //    draws its OWN soft drop shadow (a separate translucent
        //    DropShadower window layered just outside our bounds) whenever
        //    the native OS title bar is off - normally invisible, because
        //    normal windowed mode keeps the native title bar and lets
        //    Windows' own DWM shadow handle it instead. The previous
        //    fullscreen code turned the native title bar off but never
        //    turned JUCE's shadow off, so that shadow rendered right at the
        //    screen edge and looked like a leftover window border.
        //    setDropShadowEnabled(false) before entering kiosk mode is the
        //    real fix; kiosk mode alone would not have fixed this.
        void setTrueFullScreen(bool shouldBeFullScreen)
        {
            if (shouldBeFullScreen == trueFullScreen)
                return;

            trueFullScreen = shouldBeFullScreen;
            auto& desktop = juce::Desktop::getInstance();

            if (shouldBeFullScreen)
            {
                preFullScreenBounds = getBounds();
                wasMaximizedBeforeFullScreen = isFullScreen(); // native OS maximize state

                setDropShadowEnabled(false);
                desktop.setKioskModeComponent(this, false); // false = also hide the taskbar
                setAlwaysOnTop(true); // belt-and-braces against the taskbar re-asserting z-order
            }
            else
            {
                setAlwaysOnTop(false);
                desktop.setKioskModeComponent(nullptr);
                setDropShadowEnabled(true);

                if (wasMaximizedBeforeFullScreen)
                {
                    setBounds(preFullScreenBounds);
                    setFullScreen(true);
                }
                else
                {
                    setBounds(preFullScreenBounds);
                }
            }

            // Kiosk mode (like the native-title-bar toggling before it)
            // recreates the peer, which drops keyboard focus - reclaim it so
            // F11/Esc keep working immediately.
            if (content != nullptr)
                content->grabKeyboardFocus();
        }

        MainComponent* content = nullptr;
        bool trueFullScreen = false;
        bool wasMaximizedBeforeFullScreen = false;
        juce::Rectangle<int> preFullScreenBounds;
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(PulseApplication)
