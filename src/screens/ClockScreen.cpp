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

// Пульсация для ":" — 0..1
// Используем секунды + долю секунды → мягкое "дыхание"
static float colonPulse(uint32_t ms) {
    // период ~1 сек
    float t = (float)(ms % 1000) / 1000.0f;
    // sin: 0..1
    return 0.5f + 0.5f * sinf(t * 2.0f * PI);
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
} else {
    // перерисовываем ":" для пульсации
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
// drawTime — ШАГ A (ТОЛЬКО :)
// =====================================================
void ClockScreen::drawTime(bool force) {

    if (!time.isValid())
        return;

    const ThemeBlend th = themeService().interpolate(night.value());

    // ---- fade HH:MM (как было) ----
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

    tft.setTextSize(3);

    // ---- HH ----
    uint16_t hhColor =
        ThemeService::blend565(th.muted, th.fg, fadeK);

    tft.setTextColor(hhColor, th.bg);
    tft.setCursor(X0, Y0);
    tft.printf("%02d", h);

float nightK = night.value(); // 0..1
    // В Day: accent → accent
    // В Night: accent → muted

// ---- ":" (СТАБИЛЬНО ВИДИМОЕ + МЯГКАЯ ПУЛЬСАЦИЯ) ----
// Базовый цвет — ВСЕГДА видимый
uint16_t colonColor = th.accent;
// После fade добавляем мягкую пульсацию яркости
if (!fadeActive) {
    float pulse = colonPulse(millis()); // 0..1
    // Лёгкое усиление яркости, а не замена цвета
    colonColor = ThemeService::blend565(colonColor, th.fg, pulse * 1.0f);
}

tft.setTextColor(colonColor, th.bg);
tft.setCursor(X0 + 2 * DIGIT_W, Y0);
tft.print(":");

    // ---- MM ----
    uint16_t mmColor =
        ThemeService::blend565(th.muted, th.fg, fadeK);

    tft.setTextColor(mmColor, th.bg);
    tft.setCursor(X0 + 3 * DIGIT_W, Y0);
    tft.printf("%02d", m);

// ---- seconds ----
// секунды перерисовываем ТОЛЬКО при реальном изменении времени
static int lastSec = -1;
if (s != lastSec || force) {
    lastSec = s;

    const int SEC_X = X0 + TIME_W - 24;
    const int SEC_Y = Y0 + TIME_H + 4;

    tft.fillRect(SEC_X, SEC_Y, 24, 12, th.bg);
    tft.setTextSize(1);
    tft.setTextColor(th.muted, th.bg);
    tft.setCursor(SEC_X, SEC_Y);
    tft.printf("%02d", s);
}
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

    tft.setTextSize(2);
    tft.setTextColor(th.warn, th.bg);
    tft.setTextWrap(false);

    tft.setCursor(4, y);
    tft.printf("%dC", (int)round(dht.temperature()));

    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", (int)round(dht.humidity()));

    const int w = strlen(buf) * 8;
    tft.setCursor(tft.width() - w-15 , y);
    tft.print(buf);

    dhtDrawnOnce = true;
}