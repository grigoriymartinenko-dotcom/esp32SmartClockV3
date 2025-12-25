#include "screens/ClockScreen.h"

// =====================================================
// Fade config (только HH:MM)
// =====================================================
static constexpr uint8_t FADE_STEPS = 5;

// =====================================================
// Настройка расположения времени
// =====================================================
static constexpr int TIME_SHIFT_X = 0;
static constexpr int TIME_SHIFT_Y = -6;

// =====================================================
// DHT layout
// =====================================================

static constexpr int DHT_Y_OFFSET = 4;
static constexpr int DHT_ROW_H    = 12;   // 👈 НЕ DHT_H
//static constexpr int DHT_H = 12;

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
    UiVersionService& uiVer,
    DhtService& dhtService
)
    : Screen(themeService)
    , tft(t)
    , time(timeService)
    , night(nightService)
    , layout(layoutService)
    , uiVersion(uiVer)
    , dht(dhtService)
{}

// =====================================================
// begin
// =====================================================
void ClockScreen::begin() {

    uint32_t sv = uiVersion.version(UiChannel::SCREEN);
    if (sv != lastScreenV) {
        lastScreenV = sv;
        fadeActive = true;
        fadeStep = 0;
    }

    tft.fillRect(
        0,
        layout.contentY(),
        tft.width(),
        layout.contentH(),
        theme().bg
    );

    lastTimeV   = uiVersion.version(UiChannel::TIME);
    lastThemeV  = uiVersion.version(UiChannel::THEME);
    lastDhtV = uiVersion.version(UiChannel::DHT);

    drawDht(true);
}

// =====================================================
// update
// =====================================================
void ClockScreen::update() {

    // ===== FADE HH:MM =====
    if (fadeActive) {
        drawTime(true);
        lastTimeV = uiVersion.version(UiChannel::TIME);

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

        tft.fillRect(
            0,
            layout.contentY(),
            tft.width(),
            layout.contentH(),
            theme().bg
        );

        drawDht(true);
        drawTime(true);
        return;
    }

    // ===== TIME =====
    uint32_t timeV = uiVersion.version(UiChannel::TIME);
    if (timeV != lastTimeV) {
        lastTimeV = timeV;
        drawTime(false);
    }

    // ===== SENSOR (DHT) =====
uint32_t dhtV = uiVersion.version(UiChannel::DHT);

// первый валидный показ — даже если версия не менялась
if (dht.isValid() && (!dhtDrawnOnce || dhtV != lastDhtV)) {
    lastDhtV = dhtV;
    drawDht(false);
}
}

// =====================================================
// drawTime
// =====================================================
void ClockScreen::drawTime(bool force) {

    if (!time.isValid())
        return;

    uint8_t a = 255;
    if (fadeActive) {
        uint16_t t = (uint16_t)fadeStep * 255 / FADE_STEPS;
        a = (t * t) / 255;
    }

    const uint16_t timeColor = fadeActive
        ? blend565(theme().bg, theme().textPrimary, a)
        : theme().textPrimary;

    const uint16_t secColor = theme().muted;

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

    const int safeY = layout.contentY();
    const int safeH = layout.contentH();

    const int X0 = (tft.width() - TIME_W) / 2;
    const int Y0 = safeY + (safeH - TIME_H) / 2;

    const int X = X0 + TIME_SHIFT_X;
    const int Y = Y0 + TIME_SHIFT_Y;

    const int SEC_W = 24;
    const int SEC_H = 12;

    const int SEC_X = X + TIME_W - SEC_W;
    const int SEC_Y = Y + TIME_H + 4;

    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, theme().bg);
    }

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

    if (showSeconds) {
        tft.fillRect(SEC_X, SEC_Y, SEC_W, SEC_H, theme().bg);
        tft.setTextSize(1);
        tft.setTextColor(secColor, theme().bg);
        tft.setCursor(SEC_X, SEC_Y);
        tft.printf("%02d", s);
    } else if (force) {
        tft.fillRect(SEC_X, SEC_Y, SEC_W, SEC_H, theme().bg);
    }
}

// =====================================================
// drawDht
// =====================================================
void ClockScreen::drawDht(bool force) {

    const int y = layout.contentY() + DHT_Y_OFFSET;

    if (force) {
        tft.fillRect(0, y, tft.width(), DHT_ROW_H, theme().bg);
    }

    if (!dht.isValid())
        return;

    tft.setTextSize(1);
    tft.setTextColor(theme().muted, theme().bg);
    tft.setTextWrap(false);

    // Temperature (left)
    tft.setCursor(4, y);
    tft.printf("%dC", (int)round(dht.temperature()));

    // Humidity (right)
    const char buf[8] = {0};
    snprintf((char*)buf, sizeof(buf), "%d%%", (int)round(dht.humidity()));

    const int w = strlen(buf) * 6;
    tft.setCursor(tft.width() - w - 4, y);
    tft.print(buf);
    dhtDrawnOnce = true;
}