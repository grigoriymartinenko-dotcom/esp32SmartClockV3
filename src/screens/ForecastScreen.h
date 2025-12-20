#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/ThemeService.h"
#include "services/ForecastService.h"
#include "services/LayoutService.h"

/*
 * ForecastScreen
 * --------------
 * Реактивный экран прогноза (today only).
 *
 * ПРАВИЛА:
 *  - StatusBar: ДА
 *  - BottomBar: НЕТ
 *  - Нижней линии НЕТ
 *  - В begin() очищает свою область ОДИН раз
 *  - Далее рисует ТОЛЬКО изменившиеся строки (частичная перерисовка)
 */
class ForecastScreen : public Screen {
public:
    ForecastScreen(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        ForecastService& forecast,
        LayoutService& layout
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return true; }
    bool hasBottomBar() const override { return false; }

private:
    // Рисует/обновляет конкретные элементы (частично)
    void drawTitle(bool force);
    void drawNoData(bool force);
    void drawRowDay(bool force, int dayTemp);
    void drawRowNight(bool force, int nightTemp);
    void drawRowHum(bool force, int hum);

    // Утилиты
    void clearWorkArea();
    void hardClearBottom2px();
    void resetCache();
    bool themeChanged() const;

    // Экран / сервисы
    Adafruit_ST7735& _tft;
    ForecastService& _forecast;
    LayoutService&   _layout;

    // dirty для "полного" кадра после begin() или смены темы
    bool _dirty = true;

    // ==== кеш отрисованных значений (для частичной перерисовки) ====
    bool _lastReady = false;
    int  _lastDay   = -10000;
    int  _lastNight = -10000;
    int  _lastHum   = -1;

    // следим за темой (если поменялась — полный redraw)
    uint16_t _lastBg = 0;
};