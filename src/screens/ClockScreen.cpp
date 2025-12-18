#include "screens/ClockScreen.h"
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

static constexpr int SCREEN_W = 160;
static constexpr int SCREEN_H = 128;

static constexpr int TOP_LINE_Y    = 22;
static constexpr int BOTTOM_LINE_Y = 78;

// базовая линия текста времени
static constexpr int TIME_BASE_Y   = 62;

// ✔️ финальный визуальный центр (без смещения)
static constexpr int TIME_X_OFFSET = 2;

ClockScreen::ClockScreen(
    Adafruit_ST7735& t,
    TimeService& ts,
    NightService& ns,
    const Theme& th
)
: Screen(th), tft(t), time(ts), night(ns) {}

void ClockScreen::begin() {
    tft.fillScreen(theme.bg);

    // линии
    tft.drawFastHLine(0, TOP_LINE_Y, SCREEN_W, theme.accent);
    tft.drawFastHLine(0, BOTTOM_LINE_Y, SCREEN_W, theme.accent);

    lastH = lastM = lastS = -1;
}

void ClockScreen::update() {
    if (!time.isValid()) return;
    drawTime();
}

void ClockScreen::drawTime() {
    int h = time.hour();
    int m = time.minute();
    int s = time.second();

    /* ================= HH  MM ================= */
    if (h != lastH || m != lastM) {

        // чистим строго между линиями
        tft.fillRect(
            0,
            TOP_LINE_Y + 2,
            SCREEN_W,
            BOTTOM_LINE_Y - TOP_LINE_Y - 3,
            theme.bg
        );

        tft.setFont(&FreeSansBold18pt7b);
        tft.setTextColor(theme.primary, theme.bg);

        char buf[6];
        snprintf(buf, sizeof(buf), "%02d %02d", h, m);

        int16_t x1, y1;
        uint16_t w, hgt;
        tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &hgt);

        // 🎯 ЧИСТЫЙ МАТЕМАТИЧЕСКИЙ ЦЕНТР
        int x = (SCREEN_W - (int)w) / 2 + TIME_X_OFFSET;
        tft.setCursor(x, TIME_BASE_Y);
        tft.print(buf);

        lastH = h;
        lastM = m;
    }

    /* ================= : ================= */
    if (s != lastS) {

        const int colonX = SCREEN_W / 2 - 5 + TIME_X_OFFSET;
        const int colonTop = TIME_BASE_Y - 20;
        const int colonH   = 24;

        tft.fillRect(colonX, colonTop, 10, colonH, theme.bg);

        if ((s & 1) == 0) {
            tft.setFont(&FreeSansBold18pt7b);
            tft.setTextColor(theme.primary, theme.bg);
            tft.setCursor(colonX, TIME_BASE_Y);
            tft.print(":");
        }

        lastS = s;
    }

    // гарантируем линии
    tft.drawFastHLine(0, TOP_LINE_Y, SCREEN_W, theme.accent);
    tft.drawFastHLine(0, BOTTOM_LINE_Y, SCREEN_W, theme.accent);
}