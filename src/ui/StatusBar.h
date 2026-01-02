#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/ThemeBlend.h"
#include "services/TimeService.h"
#include "services/WifiService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель.
 *
 * ПРАВИЛО:
 *  - StatusBar работает ТОЛЬКО с ThemeBlend
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
        TimeService& time,
        WifiService& wifi
    );

    void update();
    void markDirty();
    //void drawTimeOnly();

private:
    void drawStatic(const ThemeBlend& th);
    void drawDot(int cx, int cy, uint16_t color);

    Status mapWifiStatus() const;
    Status mapTimeStatus() const;
    uint16_t statusDotColor(Status s, const ThemeBlend& th) const;
    const char* weekdayEnFromTm(const tm& t) const;

private:
    Adafruit_ST7735&         _tft;
    ThemeService&            _theme;
    TimeService&             _time;
    WifiService&             _wifi;

    Status _wifiSt = OFFLINE;
    Status _timeSt = OFFLINE;
private:
    bool _timeDirty = true;
    char _lastTimeStr[32] = {0};
    bool _dirty = true;
};