#include "screens/ClockScreen.h"
#include <math.h>

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
static constexpr int DHT_ROW_H    = 12;

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
    : Screen(themeService)          // 🔥 КЛЮЧЕВОЙ ФИКС
    , tft(t)
    , time(timeService)
    , night(nightTransition)
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

        fadeActive = !night.transitioning();
        fadeStep   = 0;
    }

    const float k = night.value();
    const uint16_t bg = ThemeService::blend565(
        THEME_DAY.bg,
        THEME_NIGHT.bg,
        k
    );

    tft.fillRect(
        0,
        layout.contentY(),
        tft.width(),
        layout.contentH(),
        bg
    );

    lastTimeV = uiVersion.version(UiChannel::TIME);
    lastDhtV  = uiVersion.version(UiChannel::DHT);

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

    // ===== SENSOR (DHT) =====
    uint32_t dhtV = uiVersion.version(UiChannel::DHT);
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

    const float k = night.value();

    float fadeK = 1.0f;
    if (fadeActive) {
        float t = (float)fadeStep / (float)FADE_STEPS;
        fadeK = t * t;
    }

    const uint16_t bg = ThemeService::blend565(
        THEME_DAY.bg,
        THEME_NIGHT.bg,
        k
    );

    const uint16_t text = ThemeService::blend565(
        THEME_DAY.textPrimary,
        THEME_NIGHT.textPrimary,
        k
    );

    const uint16_t muted = ThemeService::blend565(
        THEME_DAY.muted,
        THEME_NIGHT.muted,
        k
    );

    const uint16_t timeColor =
        ThemeService::blend565(muted, text, fadeK);

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();

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

    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, bg);
    }

    const bool colonVisible =
        (uiVersion.version(UiChannel::TIME) % 2) == 0;

    tft.setTextSize(3);
    tft.setTextColor(timeColor, bg);
    tft.setCursor(X, Y);

    if (colonVisible)
        tft.printf("%02d:%02d", h, m);
    else
        tft.printf("%02d %02d", h, m);

    // seconds
    const int SEC_W = 24;
    const int SEC_H = 12;
    const int SEC_X = X + TIME_W - SEC_W;
    const int SEC_Y = Y + TIME_H + 4;

    tft.fillRect(SEC_X, SEC_Y, SEC_W, SEC_H, bg);
    tft.setTextSize(1);
    tft.setTextColor(muted, bg);
    tft.setCursor(SEC_X, SEC_Y);
    tft.printf("%02d", s);
}

// =====================================================
// drawDht
// =====================================================
void ClockScreen::drawDht(bool force) {

    const float k = night.value();

    const uint16_t bg = ThemeService::blend565(
        THEME_DAY.bg,
        THEME_NIGHT.bg,
        k
    );

    const uint16_t muted = ThemeService::blend565(
        THEME_DAY.muted,
        THEME_NIGHT.muted,
        k
    );

    const int y = layout.contentY() + DHT_Y_OFFSET;

    if (force) {
        tft.fillRect(0, y, tft.width(), DHT_ROW_H, bg);
    }

    if (!dht.isValid())
        return;

    tft.setTextSize(1);
    tft.setTextColor(muted, bg);
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