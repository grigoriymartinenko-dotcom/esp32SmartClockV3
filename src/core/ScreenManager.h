#pragma once

#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"
#include "ui/UiSeparator.h"
#include "ui/UiDebugOverlay.h"
#include "services/LayoutService.h"

/*
 * ScreenManager
 * -------------
 * Управляет экранами и общими UI-элементами.
 */
class ScreenManager {
public:
    ScreenManager(
        Adafruit_ST7735& tft,
        Screen& initial,
        StatusBar& statusBar,
        BottomBar& bottomBar,
        LayoutService& layout,
        UiSeparator& sepStatus,
        UiSeparator& sepBottom
    );

    void begin();
    void update();

    void set(Screen& screen);

    bool currentHasStatusBar() const;
    bool currentHasBottomBar() const;

private:
    void applyLayout();

private:
    Adafruit_ST7735* _tft;
    Screen*          _current = nullptr;
    StatusBar*       _statusBar;
    BottomBar*       _bottomBar;
    LayoutService*   _layout;
    UiSeparator*     _sepStatus;
    UiSeparator*     _sepBottom;
};