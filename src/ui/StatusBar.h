#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"
#include "services/WifiService.h"
#include "services/NightTransitionService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель (2 строки):
 *
 *  ● WiFi        Monday  28.12.2025
 *  ● NTP / RTC
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - НЕТ логики времени
 *  - НЕТ if (night)
 *
 * КЛЮЧ:
 *  - Цвета всегда получаются через blend565()
 *  - NightTransitionService даёт ТОЛЬКО коэффициент (value)
 */
class StatusBar {
public:
    static constexpr int HEIGHT = 24;

    enum Status {
        OFFLINE,
        CONNECTING,
        ONLINE,
        ERROR
    };

    StatusBar(
        Adafruit_ST7735& tft,
        ThemeService& theme,
        NightTransitionService& nightTransition,
        TimeService& time,
        WifiService& wifi
    );

    void update();
    void markDirty();

    // вызывается ТОЛЬКО при UiChannel::TIME
    void drawTimeOnly();

private:
    // --- drawing ---
    void drawStatic();
    void drawDot(int cx, int cy, uint16_t color);

    // --- helpers ---
    Status mapWifiStatus() const;
    Status mapTimeStatus() const;
    uint16_t statusDotColor(Status s) const;
    const char* weekdayEnFromTm(const tm& t) const;

private:
    Adafruit_ST7735&       _tft;
    ThemeService&          _theme;
    NightTransitionService& _night;
    TimeService&           _time;
    WifiService&           _wifi;

    Status _wifiSt = OFFLINE;
    Status _timeSt = OFFLINE;

    // --- visual cache ---
    uint16_t _lastBg = 0;
    char     _lastTimeStr[32] = {0};

    bool _dirty = true;
};