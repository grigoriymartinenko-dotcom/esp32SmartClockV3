#pragma once
#include <stdint.h>

/*
 * SettingsTypes.h
 * ---------------
 * Типы экрана Settings вынесены отдельно:
 *  - Level / UiMode / HintBtn
 *  - MenuItem
 *
 * Это уменьшает SettingsScreen.h и убирает шум.
 *
 * ВАЖНО:
 *  - НЕ содержит никаких зависимостей от Arduino/Adafruit
 *  - Только POD-типы
 */

namespace SettingsTypes {

    enum class Level : uint8_t {
        ROOT,
        NIGHT,
        TIMEZONE,
        TIME
    };

    enum class UiMode : uint8_t {
        NAV,
        EDIT
    };

    enum class HintBtn : uint8_t {
        NONE,
        LEFT,
        RIGHT,
        OK,
        BACK
    };

    struct MenuItem {
        const char* label;
        Level target;
    };

} // namespace SettingsTypes