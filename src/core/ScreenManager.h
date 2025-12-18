#pragma once
#include "core/Screen.h"

/*
 * ScreenManager v3
 * ================
 * Управляет активным экраном
 */
class ScreenManager {
public:
    void set(Screen* screen);
    Screen* active() const;

    void begin();
    void update();

private:
    Screen* _active = nullptr;
};