#pragma once

#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"
#include "ui/UiSeparator.h"
#include "ui/UiDebugOverlay.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"

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
        UiSeparator& sepBottom,
        UiVersionService& uiVersion
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
    Screen*          _prev    = nullptr;

    StatusBar*       _statusBar;
    BottomBar*       _bottomBar;
    LayoutService*   _layout;
    UiSeparator*     _sepStatus;
    UiSeparator*     _sepBottom;
    UiVersionService* _uiVersion;
};