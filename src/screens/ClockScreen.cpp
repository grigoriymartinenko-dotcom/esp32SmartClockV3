#include "screens/ClockScreen.h"

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
{
}

void ClockScreen::begin() {

    // фон
    tft.fillRect(
        0,
        layout.clockSafeY(),
        tft.width(),
        layout.clockSafeH(),
        theme().bg
    );

// ===== FULL CLEAR CLOCK AREA =====
// Очищаем ВСЮ область от низа StatusBar до низа экрана
// чтобы не осталось хвостов после SettingsScreen
const int y0 = layout.statusY() + layout.statusH();
const int h0 = tft.height() - y0;

tft.fillRect(
    0,
    y0,
    tft.width(),
    h0,
    theme().bg
);

    lastTimeV  = uiVersion.version(UiChannel::TIME);
    lastThemeV = uiVersion.version(UiChannel::THEME);

    drawTime(true);
}

void ClockScreen::update() {

    // 🔥 Theme / Night changed
    uint32_t themeV = uiVersion.version(UiChannel::THEME);
    if (themeV != lastThemeV) {
        lastThemeV = themeV;
        themeService.setNight(night.isNight());
        drawTime(true);
        return;
    }

    if (!time.isValid())
        return;

    // 🔥 Time changed
    uint32_t timeV = uiVersion.version(UiChannel::TIME);
    if (timeV != lastTimeV) {
        lastTimeV = timeV;
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

    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, theme().bg);
    }

    tft.setTextSize(3);
    tft.setTextColor(theme().textPrimary, theme().bg);
    tft.setCursor(X, Y);
    //tft.printf("%02d:%02d", h, m);
const bool colonVisible =
    (uiVersion.version(UiChannel::TIME) % 2) == 0;

tft.setTextSize(3);
tft.setTextColor(theme().textPrimary, theme().bg);
tft.setCursor(X, Y);

if (colonVisible) {
    tft.printf("%02d:%02d", h, m);
} else {
    tft.printf("%02d %02d", h, m);
}

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