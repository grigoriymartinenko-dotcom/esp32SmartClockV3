#pragma once
#include "core/Screen.h"

/*
 * ScreenManager
 * Управляет текущим экраном
 */
class ScreenManager {
public:
    explicit ScreenManager(Screen& initial);

    void begin();
    void update();

    void set(Screen& screen);

private:
    Screen* _current = nullptr;
};