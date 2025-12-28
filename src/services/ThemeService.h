#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "theme/Theme.h"
#include "theme/Themes.h"

/*
 * ThemeService
 * ------------
 * Управляет текущей темой (day / night) и предоставляет утилиты для цветов.
 *
 * ВАЖНО:
 *  - ThemeService НЕ знает про NightTransitionService
 *  - ThemeService НЕ делает анимаций
 *  - ThemeService предоставляет только "что рисовать" (цвета), а не "когда"
 *
 * Смысл разделения:
 *  - NightService решает: ночь сейчас или день (логика режима AUTO/ON/OFF)
 *  - NightTransitionService делает плавный коэффициент 0..1 (анимация перехода)
 *  - ThemeService даёт:
 *      * текущую базовую тему (day/night)
 *      * функции смешивания цветов / тем для UI
 */

class ThemeService {
public:
    explicit ThemeService(UiVersionService& uiVersion);

    // Установить значения по умолчанию (день).
    void begin();

    // Жёстко установить режим темы (без анимации).
    // Если значение реально изменилось — будет bump(UiChannel::THEME).
    void setNight(bool night);

    bool isNight() const;

    // Текущая активная "базовая" тема (DAY или NIGHT).
    // Используется когда НЕ нужен плавный переход.
    const Theme& current() const;

    // ------------------------------------------------------------------------
    // blending helpers
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
     * Обычно k берут из NightTransitionService::value().
     */
    static uint16_t blend565(uint16_t day, uint16_t night, float k);

    /*
     * blended(k)
     * ----------
     * Смешанная тема между THEME_DAY и THEME_NIGHT.
     *
     * Это удобно для виджетов/экранов:
     *   const Theme th = themeService.blended(nightTransition.value());
     *   tft.setTextColor(th.textPrimary, th.bg);
     *
     * Возвращает копию Theme (маленькая структура из 8 uint16_t).
     */
    Theme blended(float k) const;

private:
    UiVersionService& _uiVersion;

    bool  _night = false;
    Theme _theme;
};