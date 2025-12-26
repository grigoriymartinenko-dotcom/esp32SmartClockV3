#pragma once

#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "ui/StatusBar.h"
//#include "ui/BottomBar.h"   // legacy, не используется
#include "ui/ButtonBar.h"
#include "ui/UiSeparator.h"
#include "ui/UiDebugOverlay.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
#include "services/ThemeService.h"

class ScreenManager {
public:
    ScreenManager(
        Adafruit_ST7735& tft,
        Screen& initial,
        StatusBar& statusBar,
        // BottomBar legacy — не используется
        // , _bottomBar(&bottomBar)
        ButtonBar& buttonBar,          // 🔥 ДОБАВЛЕНО
        LayoutService& layout,
        UiSeparator& sepStatus,
        UiSeparator& sepBottom,
        UiVersionService& uiVersion,
        ThemeService& themeService
    );

    void begin();
    void update();
    void set(Screen& screen);

    bool currentHasStatusBar() const;
    bool currentHasBottomBar() const;

private:
    void applyLayout();
    void clearStatusArea();

private:
    Adafruit_ST7735*  _tft;
    Screen*           _current = nullptr;
    Screen*           _prev    = nullptr;

    uint32_t _lastTimeVer   = 0;
    uint32_t _lastThemeVer  = 0;
    uint32_t _lastScreenVer = 0;

    StatusBar*        _statusBar;
    //BottomBar*        _bottomBar;   // legacy
    ButtonBar*        _buttonBar;   // 🔥 ТЕПЕРЬ РЕАЛЬНО ИСПОЛЬЗУЕТСЯ
    LayoutService*    _layout;
    UiSeparator*      _sepStatus;
    UiSeparator*      _sepBottom;
    UiVersionService* _uiVersion;
    ThemeService*     _theme;
};