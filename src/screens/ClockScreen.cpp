#include "screens/ClockScreen.h"

/*
 * ClockScreen
 * -----------
 * Рисует только "контент" часов внутри безопасной зоны (между линиями).
 * Линии (UiSeparator) рисуются отдельно и не должны затираться ClockScreen'ом.
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
    // сброс кеша отрисовки
    lastH = lastM = lastS = -1;

    // синхронизируем тему при входе на экран
    lastNight = night.isNight();
    themeService.setNight(lastNight);

    // рисуем сразу
    drawTime(true);
}

void ClockScreen::update() {

    // если ночь/день переключились — надо перерисовать полностью
    bool isNightNow = night.isNight();
    if (isNightNow != lastNight) {
        lastNight = isNightNow;
        themeService.setNight(isNightNow);
        drawTime(true);
        return;
    }

    // обычное обновление (по секунде, но без лишней перерисовки)
    drawTime(false);
}

void ClockScreen::drawTime(bool force) {

    // сброс состояния GFX (Adafruit любит "наследовать" настройки)
    tft.setFont(nullptr);
    tft.setTextWrap(false);

    if (!time.isValid()) return;

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();

    // секунды показываем только днём
    const bool showSeconds = !night.isNight();

    // если ничего не изменилось — выходим (экономим перерисовку)
    if (!force &&
        h == lastH &&
        m == lastM &&
        (!showSeconds || s == lastS)) {
        return;
    }

    lastH = h;
    lastM = m;
    lastS = s;

    // --- МЕТРИКИ (твои подобранные значения) ---
    const int DIGIT_W = 18;
    const int DIGIT_H = 24;
    const int TIME_W  = 5 * DIGIT_W;  // "HH:MM"
    const int TIME_H  = DIGIT_H;

    // --- SAFE CLOCK AREA (между линиями) ---
    const int safeY = layout.clockSafeY();
    const int safeH = layout.clockSafeH();

    // центрируем время внутри safe-области
    const int X = (tft.width() - TIME_W) / 2;
    const int Y = safeY + (safeH - TIME_H) / 2;

    // очищаем ТОЛЬКО safe-область (чтобы не затирать линии)
    tft.fillRect(
        0,
        safeY,
        tft.width(),
        safeH,
        theme().bg
    );

    // HH:MM — главный текст
    tft.setTextSize(3);
    tft.setTextColor(theme().textPrimary, theme().bg);
    tft.setCursor(X, Y);
    tft.printf("%02d:%02d", h, m);

    // секунды — приглушённые и с gap
    if (showSeconds) {
        constexpr int SEC_GAP = 12;

        tft.setTextSize(1);
        tft.setTextColor(theme().muted, theme().bg);
        tft.setCursor(X + TIME_W + SEC_GAP, Y + 6);
        tft.printf("%02d", s);
    }
}