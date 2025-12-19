#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ThemeService.h"
#include "layout/LayoutService.h"

class ClockScreen : public Screen {
public:
    ClockScreen(
        Adafruit_ST7735& tft,
        TimeService& timeService,
        NightService& nightService,
        ThemeService& themeService,
        LayoutService& layoutService
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return true; }
    bool hasBottomBar() const override { return true; }

private:
    void drawTime(bool force);

    Adafruit_ST7735& tft;
    TimeService&     time;
    NightService&    night;
    LayoutService&   layout;

    int  lastH = -1;
    int  lastM = -1;
    int  lastS = -1;
    bool lastNight = false;
};