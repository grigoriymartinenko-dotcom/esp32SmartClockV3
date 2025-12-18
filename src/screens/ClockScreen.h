#pragma once
#include <Adafruit_ST7735.h>
#include "core/Screen.h"
#include "services/TimeService.h"
#include "services/NightService.h"

class ClockScreen : public Screen {
public:
    ClockScreen(Adafruit_ST7735& tft, TimeService& time, NightService& night, const Theme& theme);
    void begin() override;
    void update() override;

private:
    Adafruit_ST7735& tft;
    TimeService& time;
    NightService& night;

    int lastH = -1;
    int lastM = -1;
    int lastS = -1;

    static constexpr int W = 160;
    static constexpr int TIME_Y = 48;
    static constexpr int SEC_Y = 22;
    static constexpr int LINE_Y = 64;

    void drawTime();
};
