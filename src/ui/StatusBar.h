#pragma once
#include <Adafruit_ST7735.h>

#include "services/ThemeService.h"
#include "services/TimeService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель (2 строки):
 *
 *  ● WiFi        DD.MM.YYYY
 *  ● NTP         weekday
 *
 * ПРАВИЛА:
 *  - НЕТ таймеров
 *  - НЕТ millis()
 *  - Рисует ТОЛЬКО по dirty-флагу
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
        TimeService& time
    );

    void update();
    void markDirty();

    void setWiFiStatus(Status s);
    void setNtpStatus(Status s);

private:
    void draw();

    // helpers
    uint16_t statusDotColor(Status s, const Theme& th) const;
    const char* weekdayUaLatFromTm(const tm& t) const;

    // 🔹 НОВОЕ: рисование индикатора
    void drawDot(int cx, int cy, uint16_t color);

private:
    Adafruit_ST7735& _tft;
    ThemeService&    _theme;
    TimeService&     _time;

    Status _wifi = OFFLINE;
    Status _ntp  = OFFLINE;

    bool _dirty = true;
};