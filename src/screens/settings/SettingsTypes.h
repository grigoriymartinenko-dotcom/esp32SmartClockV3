#pragma once
#include <stdint.h>

/*
 * SettingsTypes.h
 * ---------------
 * БАЗОВЫЕ ТИПЫ экрана Settings.
 *
 * ЗАЧЕМ:
 *  - уменьшить SettingsScreen.h
 *  - убрать enum-ы и POD-структуры из логики экрана
 *  - сделать типы переиспользуемыми
 *
 * ПРАВИЛА:
 *  - НИКАКОЙ логики
 *  - НИКАКИХ Arduino / Adafruit зависимостей
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
        NAV,        // навигация
        EDIT        // редактирование значения
    };

    /*
     * HintBtn
     * -------
     * Используется ТОЛЬКО для визуального фидбека кнопок
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
     * Описание пункта корневого меню
     */
    struct MenuItem {
        const char* label;
        Level       target;
    };

} // namespace SettingsTypes