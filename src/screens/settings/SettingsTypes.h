#pragma once
#include <stdint.h>

/*
 * SettingsTypes.h
 * ---------------
 * БАЗОВЫЕ ТИПЫ экрана Settings.
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ логики
 *  - ТОЛЬКО POD-типы
 */

namespace SettingsTypes {

    /*
     * Level
     * -----
     * Текущий уровень экрана Settings
     */
    enum class Level : uint8_t {
        ROOT,       // корневое меню
        WIFI,
        WIFI_LIST,     // 🔥 НОВОЕ: список сетей
        NIGHT,      // настройки ночного режима
        TIMEZONE,   // таймзона / DST
        TIME        // источник времени
    };

    /*
     * UiMode
     * ------
     * Режим взаимодействия
     */
    enum class UiMode : uint8_t {
        NAV,
        EDIT
    };

    /*
     * HintBtn
     * -------
     * Визуальный фидбек кнопок
     */
    enum class HintBtn : uint8_t {
        NONE,
        LEFT,
        RIGHT,
        OK,
        BACK
    };

    /*
     * MenuItem
     * --------
     * Пункт корневого меню
     */
    struct MenuItem {
        const char* label;
        Level       target;
    };

} // namespace SettingsTypes