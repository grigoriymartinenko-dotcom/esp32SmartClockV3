#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/ThemeService.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
#include "services/DhtService.h"

class ClockScreen : public Screen {
public:
    ClockScreen(
        Adafruit_ST7735& tft,
        TimeService& timeService,
        NightService& nightService,
        ThemeService& themeService,
        LayoutService& layoutService,
        UiVersionService& uiVersion,
        DhtService& dhtService
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return true; }
    //bool hasBottomBar() const override { return true; }

private:
    void drawTime(bool force);
    void drawDht(bool force);

private:
    Adafruit_ST7735& tft;
    TimeService&     time;
    NightService&    night;
    LayoutService&   layout;
    UiVersionService& uiVersion;
    DhtService&      dht;

    uint32_t lastTimeV   = 0;
    uint32_t lastThemeV  = 0;
    uint32_t lastScreenV = 0;
    uint32_t lastDhtV    = 0;
    uint32_t lastSensorV    = 0;

bool dhtDrawnOnce = false;
    uint8_t  fadeStep   = 0;
    bool     fadeActive = false;
};