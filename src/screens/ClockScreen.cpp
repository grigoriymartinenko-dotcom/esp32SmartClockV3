#include "screens/ClockScreen.h"

/*
 * ClockScreen
 * -----------
 * РЕАКТИВНЫЙ экран часов.
 * Без дрожания: HH:MM и секунды рисуются в отдельных зонах.
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
    lastH = lastM = lastS = -1;

    lastNight = night.isNight();
    themeService.setNight(lastNight);

    // --------------------------------------------------
    // 1. Очистка зон, которыми владеет ClockScreen
    // --------------------------------------------------

    // safe-зона часов
    tft.fillRect(
        0,
        layout.clockSafeY(),
        tft.width(),
        layout.clockSafeH(),
        theme().bg
    );

    // зона BottomBar (если пришли с экрана без него)
    if (hasBottomBar()) {
        tft.fillRect(
            0,
            layout.bottomY(),
            tft.width(),
            layout.bottomH(),
            theme().bg
        );
    }

    // ST7735 HARD FIX (низ экрана)
    tft.fillRect(
        0,
        tft.height() - 2,
        tft.width(),
        2,
        theme().bg
    );

    // --------------------------------------------------
    // 🔥 КЛЮЧЕВОЙ ФИКС:
    // поглощаем pending-события времени,
    // чтобы update() не сделал второй redraw
    // --------------------------------------------------
    (void)time.hourChanged();
    (void)time.minuteChanged();
    (void)time.secondChanged();

    // первичная отрисовка
    drawTime(true);
}

void ClockScreen::update() {

    // --- смена день / ночь ---
    bool isNightNow = night.isNight();
    if (isNightNow != lastNight) {
        lastNight = isNightNow;
        themeService.setNight(isNightNow);

        // при смене темы тоже синхронизируем время
        (void)time.hourChanged();
        (void)time.minuteChanged();
        (void)time.secondChanged();

        drawTime(true);
        return;
    }

    if (!time.isValid()) return;

    // HH:MM
    if (time.hourChanged() || time.minuteChanged()) {
        drawTime(true);
        return;
    }

    // секунды (только днём)
    if (!night.isNight() && time.secondChanged()) {
        drawTime(false);
    }
}

/*
 * drawTime(force)
 * ----------------
 * force = true  -> перерисовать HH:MM + секунды
 * force = false -> перерисовать ТОЛЬКО секунды
 */
void ClockScreen::drawTime(bool force) {

    tft.setFont(nullptr);
    tft.setTextWrap(false);

    if (!time.isValid()) return;

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();
    const bool showSeconds = !night.isNight();

    const int DIGIT_W = 18;
    const int DIGIT_H = 24;
    const int TIME_W  = 5 * DIGIT_W;   // HH:MM
    const int TIME_H  = DIGIT_H;

    const int safeY = layout.clockSafeY();
    const int safeH = layout.clockSafeH();

    const int X = (tft.width() - TIME_W) / 2;
    const int Y = safeY + (safeH - TIME_H) / 2;

    constexpr int SEC_GAP = 12;
    const int SEC_X = X + TIME_W + SEC_GAP;
    const int SEC_Y = Y + 6;

    // ---------- HH:MM ----------
    if (force || h != lastH || m != lastM) {

        tft.fillRect(
            X,
            Y,
            TIME_W,
            TIME_H,
            theme().bg
        );

        tft.setTextSize(3);
        tft.setTextColor(theme().textPrimary, theme().bg);
        tft.setCursor(X, Y);
        tft.printf("%02d:%02d", h, m);

        lastH = h;
        lastM = m;

        // при перерисовке HH:MM секунды обновляем заново
        lastS = -1;
    }

    // ---------- СЕКУНДЫ ----------
    if (showSeconds && s != lastS) {

        tft.fillRect(
            SEC_X,
            SEC_Y,
            24,
            12,
            theme().bg
        );

        tft.setTextSize(1);
        tft.setTextColor(theme().muted, theme().bg);
        tft.setCursor(SEC_X, SEC_Y);
        tft.printf("%02d", s);

        lastS = s;
    }

    // если секунды скрылись (ночь) — стереть их один раз
    if (!showSeconds && lastS != -1) {
        tft.fillRect(
            SEC_X,
            SEC_Y,
            24,
            12,
            theme().bg
        );
        lastS = -1;
    }
}