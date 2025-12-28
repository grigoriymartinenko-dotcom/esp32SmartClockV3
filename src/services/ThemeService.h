#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "theme/Themes.h"

/*
 * ThemeService
 * ------------
 * Управляет текущей темой (day / night).
 *
 * ВАЖНО:
 *  - ThemeService НЕ знает про NightTransitionService
 *  - ThemeService НЕ делает анимаций
 *  - ThemeService предоставляет ЦВЕТА и утилиты
 *
 * NightTransitionService.value() используется СНАРУЖИ
 * (в UI / Screen), а не здесь.
 *
 * Любая логическая смена темы → bump UiChannel::THEME
 */

class ThemeService {
public:
    explicit ThemeService(UiVersionService& uiVersion);

    void begin();

    // ------------------------------------------------------------------------
    // logical theme state (жёсткое day / night)
    // ------------------------------------------------------------------------

    void setNight(bool night);
    bool isNight() const;

    // Текущая активная тема (DAY или NIGHT)
    const Theme& current() const;

    // ------------------------------------------------------------------------
    // color helpers
    // ------------------------------------------------------------------------

    /*
     * blend565
     * --------
     * Универсальная функция смешивания цветов RGB565.
     *
     * day   — цвет дневной темы
     * night — цвет ночной темы
     * k     — коэффициент (0.0 .. 1.0)
     *
     *  k = 0.0 -> day
     *  k = 1.0 -> night
     *
     * Используется В UI, совместно с NightTransitionService::value().
     *
     * ThemeService — правильное место для этой функции,
     * т.к. она относится к работе с цветами, а не к логике UI.
     */
    static uint16_t blend565(uint16_t day, uint16_t night, float k);

private:
    UiVersionService& _uiVersion;

    bool  _night = false;
    Theme _theme;
};