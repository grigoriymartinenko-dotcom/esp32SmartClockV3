#include "screens/ClockScreen.h"

// =====================================================
// Fade config (только HH:MM)
// =====================================================
static constexpr uint8_t FADE_STEPS = 30;

// =====================================================
// RGB565 blend
// =====================================================
static uint16_t blend565(uint16_t bg, uint16_t fg, uint8_t a) {
    uint8_t br = (bg >> 11) & 0x1F;
    uint8_t bgc = (bg >> 5) & 0x3F;
    uint8_t bb = bg & 0x1F;

    uint8_t fr = (fg >> 11) & 0x1F;
    uint8_t fg_c = (fg >> 5) & 0x3F;
    uint8_t fb = fg & 0x1F;

    uint8_t r = (br * (255 - a) + fr * a) / 255;
    uint8_t g = (bgc * (255 - a) + fg_c * a) / 255;
    uint8_t b = (bb * (255 - a) + fb * a) / 255;

    return (r << 11) | (g << 5) | b;
}

// =====================================================
// ctor
// =====================================================
ClockScreen::ClockScreen(
    Adafruit_ST7735& t,
    TimeService& timeService,
    NightService& nightService,
    ThemeService& themeService,
    LayoutService& layoutService,
    UiVersionService& uiVer
)
    : Screen(themeService)
    , tft(t)
    , time(timeService)
    , night(nightService)
    , layout(layoutService)
    , uiVersion(uiVer)
{}

// =====================================================
// begin
// =====================================================
void ClockScreen::begin() {

    // fade запускается только при смене экрана
    uint32_t sv = uiVersion.version(UiChannel::SCREEN);
    if (sv != lastScreenV) {
        lastScreenV = sv;
        fadeActive = true;
        fadeStep = 0;
    }

    // очистка области часов
    tft.fillRect(
        0,
        layout.statusY() + layout.statusH(),
        tft.width(),
        tft.height(),
        theme().bg
    );

    lastTimeV  = uiVersion.version(UiChannel::TIME);
    lastThemeV = uiVersion.version(UiChannel::THEME);
}

// =====================================================
// update
// =====================================================
void ClockScreen::update() {

    // ===== FADE (только HH:MM) =====
    if (fadeActive) {
        drawTime(true);
        fadeStep++;

        if (fadeStep >= FADE_STEPS) {
            fadeActive = false;
        }
        return;
    }

    // ===== THEME =====
    uint32_t themeV = uiVersion.version(UiChannel::THEME);
    if (themeV != lastThemeV) {
        lastThemeV = themeV;
        themeService.setNight(night.isNight());
        drawTime(true);
        return;
    }

    // ===== TIME =====
    uint32_t timeV = uiVersion.version(UiChannel::TIME);
    if (timeV != lastTimeV) {
        lastTimeV = timeV;
        drawTime(false);
    }
}

// =====================================================
// drawTime
// =====================================================
void ClockScreen::drawTime(bool force) {

    if (!time.isValid())
        return;

    // ===== fade alpha (ТОЛЬКО для HH:MM) =====
    uint8_t a = 255;
    if (fadeActive) {
        uint16_t t = (uint16_t)fadeStep * 255 / FADE_STEPS;
        a = (t * t) / 255;   // мягкий ease-in
    }

    // HH:MM — с fade
    uint16_t timeColor = fadeActive
        ? blend565(theme().bg, theme().textPrimary, a)
        : theme().textPrimary;

    // секунды — ВСЕГДА нормальный цвет
    uint16_t secColor = theme().muted;

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

    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, theme().bg);
    }

    // ===== HH:MM =====
    const bool colonVisible =
        (uiVersion.version(UiChannel::TIME) % 2) == 0;

    tft.setTextSize(3);
    tft.setTextColor(timeColor, theme().bg);
    tft.setCursor(X, Y);

    if (colonVisible) {
        tft.printf("%02d:%02d", h, m);
    } else {
        tft.printf("%02d %02d", h, m);
    }

    // ===== seconds (без fade) =====
    if (showSeconds) {
        tft.fillRect(SEC_X, SEC_Y, 24, 12, theme().bg);
        tft.setTextSize(1);
        tft.setTextColor(secColor, theme().bg);
        tft.setCursor(SEC_X, SEC_Y);
        tft.printf("%02d", s);
    } else if (force) {
        tft.fillRect(SEC_X, SEC_Y, 24, 12, theme().bg);
    }
}