#pragma once

#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include "ui/Theme.h"

/*
 * StatusBar
 * =========
 * Верхняя строка статусов:
 *  WiFi | NTP | Night
 *
 * Получает состояния извне (через setXXX()),
 * не знает про сервисы, не мигает.
 */
class StatusBar {
public:
    explicit StatusBar(Adafruit_ST7735& tft);

    void setWiFi(bool connected);
    void setNtp(bool synced);
    void setNight(bool night); 

    void draw(int x, int y, int w, int h, bool force = false);

private:
    Adafruit_ST7735& _tft;

    bool _wifi  = false;
    bool _ntp   = false;
    bool _night = false;

    bool _lw = false, _ln = false, _lni = false;
    bool _first = true;

    int _x{}, _y{}, _w{}, _h{};

    void drawBg();
    void drawWiFi(int x, int y, bool ok);
    void drawNtp(int x, int y, bool ok);
    void drawNight(int x, int y, bool on);
};