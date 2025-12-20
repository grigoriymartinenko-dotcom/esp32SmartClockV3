#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ThemeService.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
class ClockScreen : public Screen {
public:
    ClockScreen(
        Adafruit_ST7735& tft,
        TimeService& timeService,
        NightService& nightService,
        ThemeService& themeService,
        LayoutService& layoutService,
        UiVersionService& uiVersion
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
    UiVersionService& uiVersion;

    uint32_t lastTimeV   = 0;
    uint32_t lastThemeV  = 0;
    uint32_t lastScreenV = 0;

    uint8_t  fadeStep   = 0;
    bool     fadeActive = false;
};