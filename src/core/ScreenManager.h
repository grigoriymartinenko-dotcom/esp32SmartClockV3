#pragma once

#include "core/Screen.h"
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"
#include "ui/UiSeparator.h"
#include "services/LayoutService.h"

/*
 * ScreenManager
 * -------------
 * Единственная точка управления:
 *  - активным экраном
 *  - layout (есть ли BottomBar)
 *  - визуальными разделителями (UiSeparator)
 *
 * main.cpp НИЧЕГО про линии не знает.
 */
class ScreenManager {
public:
    ScreenManager(
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
    void applyLayout();   // ← ключевая функция

private:
    Screen*        _current = nullptr;
    StatusBar*     _statusBar;
    BottomBar*     _bottomBar;
    LayoutService* _layout;
    UiSeparator*   _sepStatus;
    UiSeparator*   _sepBottom;
};