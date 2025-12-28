#include "screens/ClockScreen.h"
#include <math.h>

// =====================================================
// Fade config (HH:MM)
// =====================================================
static constexpr uint8_t FADE_STEPS = 6;

// =====================================================
// Time layout
// =====================================================
static constexpr int TIME_SHIFT_X = 0;
static constexpr int TIME_SHIFT_Y = -6;

// =====================================================
// DHT layout
// =====================================================
static constexpr int DHT_Y_OFFSET = 4;
static constexpr int DHT_ROW_H    = 12;

// =====================================================
// helpers
// =====================================================
static float smooth01(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

// =====================================================
// ctor
// =====================================================
ClockScreen::ClockScreen(
    Adafruit_ST7735& t,
    TimeService& timeService,
    NightTransitionService& nightTransition,
    ThemeService& themeService,
    LayoutService& layoutService,
    UiVersionService& uiVer,
    DhtService& dhtService
)
    : Screen(themeService)
    , tft(t)
    , time(timeService)
    , night(nightTransition)
    , layout(layoutService)
    , uiVersion(uiVer)
    , dht(dhtService)
{
}

// =====================================================
// begin
// =====================================================
void ClockScreen::begin() {

    uint32_t sv = uiVersion.version(UiChannel::SCREEN);
    if (sv != lastScreenV) {
        lastScreenV = sv;
        fadeActive = true;
        fadeStep   = 0;
    }

    const ThemeBlend th = themeService().interpolate(night.value());

    // Очистка рабочей области
    tft.fillRect(
        0,
        layout.contentY(),
        tft.width(),
        layout.contentH(),
        th.bg
    );

    lastTimeV = uiVersion.version(UiChannel::TIME);
    lastDhtV  = uiVersion.version(UiChannel::DHT);

    drawTime(true);
    drawDht(true);
}

// =====================================================
// update
// =====================================================
void ClockScreen::update() {

    // ===== NIGHT TRANSITION =====
    if (night.dirty()) {
        night.clearDirty();
        drawTime(true);
        drawDht(true);
        return;
    }

    // ===== FADE HH:MM =====
    if (fadeActive) {
        drawTime(true);
        fadeStep++;
        if (fadeStep >= FADE_STEPS) {
            fadeActive = false;
        }
        return;
    }

    // ===== TIME =====
    uint32_t timeV = uiVersion.version(UiChannel::TIME);
    if (timeV != lastTimeV) {
        lastTimeV = timeV;
        drawTime(false);
    }

    // ===== DHT =====
    uint32_t dhtV = uiVersion.version(UiChannel::DHT);
    if (dht.isValid() && (!dhtDrawnOnce || dhtV != lastDhtV)) {
        lastDhtV = dhtV;
        drawDht(false);
    }
}

// =====================================================
// drawTime (premium look)
// =====================================================
void ClockScreen::drawTime(bool force) {

    if (!time.isValid())
        return;

    const ThemeBlend th = themeService().interpolate(night.value());

    // ---- fade ----
    float fadeK = 1.0f;
    if (fadeActive) {
        fadeK = smooth01((float)fadeStep / (float)FADE_STEPS);
    }

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();

    const int DIGIT_W = 18;
    const int DIGIT_H = 24;
    const int TIME_W  = 5 * DIGIT_W;
    const int TIME_H  = DIGIT_H;

    const int X0 = (tft.width() - TIME_W) / 2 + TIME_SHIFT_X;
    const int Y0 = layout.contentY()
                 + (layout.contentH() - TIME_H) / 2
                 + TIME_SHIFT_Y;

    if (force) {
        tft.fillRect(X0, Y0, TIME_W, TIME_H, th.bg);
    }

    const bool colonVisible =
        (uiVersion.version(UiChannel::TIME) % 2) == 0;

    tft.setTextSize(3);

    // ---- shadow (pseudo depth) ----
    tft.setTextColor(th.muted, th.bg);
    tft.setCursor(X0 + 1, Y0 + 1);
    tft.printf("%02d:%02d", h, m);

    // ---- HH ----
    uint16_t hhColor =
        ThemeService::blend565(th.muted, th.fg, fadeK);

    tft.setTextColor(hhColor, th.bg);
    tft.setCursor(X0, Y0);
    tft.printf("%02d", h);

    // ---- colon ----
    uint16_t colonColor =
        ThemeService::blend565(th.muted, th.accent, fadeK);

    tft.setTextColor(colonColor, th.bg);
    tft.setCursor(X0 + 2 * DIGIT_W, Y0);
    tft.print(colonVisible ? ":" : " ");

    // ---- MM ----
    uint16_t mmColor =
        ThemeService::blend565(th.muted, th.fg, fadeK * 0.85f);

    tft.setTextColor(mmColor, th.bg);
    tft.setCursor(X0 + 3 * DIGIT_W, Y0);
    tft.printf("%02d", m);

    // ---- seconds ----
    const int SEC_X = X0 + TIME_W - 24;
    const int SEC_Y = Y0 + TIME_H + 4;

    tft.fillRect(SEC_X, SEC_Y, 24, 12, th.bg);
    tft.setTextSize(1);
    tft.setTextColor(th.muted, th.bg);
    tft.setCursor(SEC_X, SEC_Y);
    tft.printf("%02d", s);
}

// =====================================================
// drawDht
// =====================================================
void ClockScreen::drawDht(bool force) {

    const ThemeBlend th = themeService().interpolate(night.value());
    const int y = layout.contentY() + DHT_Y_OFFSET;

    if (force) {
        tft.fillRect(0, y, tft.width(), DHT_ROW_H, th.bg);
    }

    if (!dht.isValid())
        return;

    tft.setTextSize(1);
    tft.setTextColor(th.muted, th.bg);
    tft.setTextWrap(false);

    tft.setCursor(4, y);
    tft.printf("%d°C", (int)round(dht.temperature()));

    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", (int)round(dht.humidity()));

    const int w = strlen(buf) * 6;
    tft.setCursor(tft.width() - w - 4, y);
    tft.print(buf);

    dhtDrawnOnce = true;
}