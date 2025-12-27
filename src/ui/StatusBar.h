#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"
#include "services/WifiService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель (2 строки):
 *
 *  ● WiFi        DD.MM.YYYY
 *  ● NTP / RTC   weekday
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - НЕТ вызовов из экранов
 *  - Состояние читается ТОЛЬКО из сервисов
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

private:
    void draw();

    Status mapWifiStatus() const;
    Status mapTimeStatus() const;

    uint16_t statusDotColor(Status s, const Theme& th) const;
    const char* weekdayUaLatFromTm(const tm& t) const;

    void drawDot(int cx, int cy, uint16_t color);

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    TimeService&     _time;
    WifiService&     _wifi;

    Status _wifiSt = OFFLINE;
    Status _timeSt = OFFLINE;

    bool _dirty = true;
};