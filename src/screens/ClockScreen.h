#pragma once

#include <Arduino.h>
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "core/TimeService.h"
#include "core/NightService.h"
#include "ui/Theme.h"

class ClockScreen : public Screen {
public:
    ClockScreen(Adafruit_ST7735& tft, TimeService& time, NightService& night);

    void begin() override;
    void update() override;

private:
    Adafruit_ST7735& _tft;
    TimeService& _time;
    NightService& _night;

    int _lastH = -1;
    int _lastM = -1;
    int _lastS = -1;

    bool _colonVisible = true;
    unsigned long _lastBlinkMs = 0;

    // layout
    static constexpr int STATUS_H = 12;
    static constexpr int TIME_TOP = 18;
    static constexpr int TIME_H   = 40;
    static constexpr int SEP1_Y   = 12;
    static constexpr int SEP2_Y   = 70;

    void drawBackground();
    void drawSeparators();
    void drawTime(bool force);
};