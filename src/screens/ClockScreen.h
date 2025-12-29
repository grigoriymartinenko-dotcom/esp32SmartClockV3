#pragma once

#include <Adafruit_ST7735.h>

#include "core/Screen.h"

#include "services/TimeService.h"
#include "services/NightTransitionService.h"
#include "services/ThemeService.h"
#include "services/LayoutService.h"
#include "services/UiVersionService.h"
#include "services/DhtService.h"

/*
 * ClockScreen
 * -----------
 * Главный экран часов.
 *
 * АРХИТЕКТУРА:
 *  - Экран НЕ знает про Day/Night
 *  - Экран работает ТОЛЬКО с ThemeBlend
 *
 * UX:
 *  - Пульсирующее двоеточие (:)
 *  - HH / MM не затрагиваются (Шаг A)
 */

class ClockScreen : public Screen {
public:
    ClockScreen(
        Adafruit_ST7735&        tft,
        TimeService&            timeService,
        NightTransitionService& nightTransition,
        ThemeService&           themeService,
        LayoutService&          layoutService,
        UiVersionService&       uiVersion,
        DhtService&             dhtService
    );

    void begin() override;
    void update() override;

private:
    void drawTime(bool force);
    void drawDht(bool force);

private:
    Adafruit_ST7735&        tft;
    TimeService&            time;
    NightTransitionService& night;
    LayoutService&          layout;
    UiVersionService&       uiVersion;
    DhtService&             dht;

    uint32_t lastTimeV    = 0;
    uint32_t lastDhtV     = 0;
    uint32_t lastScreenV  = 0;

    bool     dhtDrawnOnce = false;

    // fade HH:MM (оставляем как было)
    bool     fadeActive   = false;
    uint8_t  fadeStep     = 0;
};