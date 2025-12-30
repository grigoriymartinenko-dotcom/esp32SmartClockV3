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
        ROOT,           // корневое меню
        WIFI,           // Wi-Fi ON / OFF / Scan
        WIFI_LIST,      // список сетей
        WIFI_PASSWORD,  // 🔥 ВВОД ПАРОЛЯ
        NIGHT,
        TIMEZONE,
        TIME,
        BRIGHTNESS
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