#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"

/*
 * ============================================================
 * StatusBar — верхняя строка состояния
 *
 * Отображает:
 *  - Wi-Fi (W)
 *  - валидность времени / NTP (N)
 *  - день недели + дату
 *
 * ПРАВИЛА:
 *  - сам владеет своей зоной экрана
 *  - НИКОГДА не затирает область ниже
 *  - перерисовывается ТОЛЬКО при изменении состояния
 * ============================================================
 */

/* ---------- Геометрия статусбара (ЕДИНСТВЕННЫЙ ИСТОЧНИК) ---------- */
static constexpr int STATUS_BAR_H = 24;

static constexpr int ICON_Y = 16;
static constexpr int WIFI_X = 6;
static constexpr int NTP_X  = 22;
static constexpr int DATE_X = 42;

class StatusBar {
public:
    StatusBar(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        TimeService& time
    );

    void setWiFi(bool connected);
    void draw();

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    TimeService&     _time;

    // текущее состояние
    bool _wifiOk = false;

    // предыдущее состояние (для dirty-check)
    bool _lastWifiOk   = false;
    bool _lastTimeOk   = false;
    bool _lastIsNight  = false;
    bool _wasTimeValid = false;

    int  _lastDay   = -1;
    int  _lastMonth = -1;
    int  _lastYear  = -1;

    bool isDirty() const;

    void drawBackground();
    void drawIcons();
    void drawDate();
};