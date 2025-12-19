#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/TimeService.h"
#include "services/NightService.h"
#include "services/DhtService.h"

class ClockScreen : public Screen {
public:
    ClockScreen(
        Adafruit_ST7735& tft,
        TimeService& time,
        NightService& night,
        ThemeService& themeService
    );

    void begin() override;
    void update() override;
int lastSecShown = -1;
// последние отрисованные значения нижней панели
int lastTempShown = -10000;  // *10 (22.3 → 223)
int lastHumShown  = -1;
private:

DhtService dht;

uint32_t lastDhtUpdateMs = 0;

    Adafruit_ST7735& tft;
    TimeService&     time;
    NightService&    night;

    int lastH = -1;
    int lastM = -1;
    int lastS = -1;
void drawDht();   // отдельная отрисовка
    void drawTime();
};