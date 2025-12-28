#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/ThemeBlend.h"
#include "services/NightTransitionService.h"
#include "services/ColorTemperatureService.h"
#include "services/TimeService.h"
#include "services/WifiService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель.
 *
 * ПРАВИЛО (НОВОЕ):
 *  - StatusBar работает ТОЛЬКО с ThemeBlend
 *  - НЕТ Day/Night
 *  - НЕТ blend565
 *  - NightTransition → коэффициент
 *  - ColorTemperature → пост-фильтр
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
        ColorTemperatureService& colorTemp,
        TimeService& time,
        WifiService& wifi
    );

    void update();
    void markDirty();
    void drawTimeOnly();

private:
    void drawStatic(const ThemeBlend& th);
    void drawDot(int cx, int cy, uint16_t color);

    Status mapWifiStatus() const;
    Status mapTimeStatus() const;
    uint16_t statusDotColor(Status s, const ThemeBlend& th) const;
    const char* weekdayEnFromTm(const tm& t) const;

private:
    Adafruit_ST7735&        _tft;
    ThemeService&           _theme;
    NightTransitionService& _night;
    ColorTemperatureService& _temp;
    TimeService&            _time;
    WifiService&            _wifi;

    Status _wifiSt = OFFLINE;
    Status _timeSt = OFFLINE;

    char _lastTimeStr[32] = {0};
    bool _dirty = true;
};