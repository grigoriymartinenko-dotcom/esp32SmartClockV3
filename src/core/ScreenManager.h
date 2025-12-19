#pragma once
#include "core/Screen.h"
#include "ui/StatusBar.h"
#include "ui/BottomBar.h"

class ScreenManager {
public:
    ScreenManager(
        Screen& initial,
        StatusBar& statusBar,
        BottomBar& bottomBar
    );

    void begin();
    void update();

    void set(Screen& screen);

    bool currentHasStatusBar() const;
    bool currentHasBottomBar() const;

private:
    Screen*    _current = nullptr;
    StatusBar* _statusBar;
    BottomBar* _bottomBar;
};