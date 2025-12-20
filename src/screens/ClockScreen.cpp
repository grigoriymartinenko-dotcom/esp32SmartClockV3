#include "screens/ClockScreen.h"

/*
 * ClockScreen
 * -----------
 * РЕАКТИВНЫЙ экран часов.
 * Реакция ТОЛЬКО на изменение версии TimeService.
 */

ClockScreen::ClockScreen(
    Adafruit_ST7735& t,
    TimeService& timeService,
    NightService& nightService,
    ThemeService& themeService,
    LayoutService& layoutService
)
    : Screen(themeService)
    , tft(t)
    , time(timeService)
    , night(nightService)
    , layout(layoutService)
{
}

void ClockScreen::begin() {
    lastNight = night.isNight();
    themeService.setNight(lastNight);

    // очистка зоны часов
    tft.fillRect(
        0,
        layout.clockSafeY(),
        tft.width(),
        layout.clockSafeH(),
        theme().bg
    );

    // очистка зоны BottomBar
    if (hasBottomBar()) {
        tft.fillRect(
            0,
            layout.bottomY(),
            tft.width(),
            layout.bottomH(),
            theme().bg
        );
    }

    // ST7735 HARD FIX (низ)
    tft.fillRect(
        0,
        tft.height() - 2,
        tft.width(),
        2,
        theme().bg
    );

    lastTimeVersion = time.version().value;
    drawTime(true);
}

void ClockScreen::update() {

    // смена день / ночь
    bool isNightNow = night.isNight();
    if (isNightNow != lastNight) {
        lastNight = isNightNow;
        themeService.setNight(isNightNow);
        drawTime(true);
        return;
    }

    if (!time.isValid())
        return;

    // 🔥 РЕАКТИВНОСТЬ ПО VERSION
    if (lastTimeVersion != time.version().value) {
        lastTimeVersion = time.version().value;
        drawTime(false);
    }
}

void ClockScreen::drawTime(bool force) {

    if (!time.isValid())
        return;

    tft.setFont(nullptr);
    tft.setTextWrap(false);

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();
    const bool showSeconds = !night.isNight();

    const int DIGIT_W = 18;
    const int DIGIT_H = 24;
    const int TIME_W  = 5 * DIGIT_W;
    const int TIME_H  = DIGIT_H;

    const int safeY = layout.clockSafeY();
    const int safeH = layout.clockSafeH();

    const int X = (tft.width() - TIME_W) / 2;
    const int Y = safeY + (safeH - TIME_H) / 2;

    constexpr int SEC_GAP = 12;
    const int SEC_X = X + TIME_W + SEC_GAP;
    const int SEC_Y = Y + 6;

    // HH:MM
    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, theme().bg);
    }

    tft.setTextSize(3);
    tft.setTextColor(theme().textPrimary, theme().bg);
    tft.setCursor(X, Y);
    tft.printf("%02d:%02d", h, m);

    // секунды
    if (showSeconds) {
        tft.fillRect(SEC_X, SEC_Y, 24, 12, theme().bg);
        tft.setTextSize(1);
        tft.setTextColor(theme().muted, theme().bg);
        tft.setCursor(SEC_X, SEC_Y);
        tft.printf("%02d", s);
    } else if (force) {
        tft.fillRect(SEC_X, SEC_Y, 24, 12, theme().bg);
    }
}