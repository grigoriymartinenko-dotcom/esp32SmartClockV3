#pragma once
#include <Adafruit_ST7735.h>
#include "services/ThemeService.h"
#include "services/TimeService.h"

/*
 * StatusBar
 * ---------
 * Верхняя статусная панель:
 * Wi-Fi / дата / NTP
 *
 * Состояния:
 *  OFFLINE     -> '-' серый
 *  CONNECTING  -> '*' синий, мигающий
 *  ONLINE      -> '+' серый
 *  ERROR       -> '!' красный
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

    void draw();
    void markDirty();

    void setWiFiStatus(Status s);
    void setNtpStatus(Status s);

private:
    Adafruit_ST7735& _tft;
    ThemeService&   _theme;
    TimeService&    _time;

    Status _wifi = OFFLINE;
    Status _ntp  = OFFLINE;

    bool _dirty = true;
};