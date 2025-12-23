#include "screens/ClockScreen.h"

// =====================================================
// Fade config (только HH:MM)
// =====================================================
static constexpr uint8_t FADE_STEPS = 5;

// =====================================================
// Настройка расположения времени (то, что ты хотел)
//
// Меняешь эти 2 числа — и HH:MM двигается.
//  - TIME_SHIFT_Y < 0  -> вверх
//  - TIME_SHIFT_Y > 0  -> вниз
//  - TIME_SHIFT_X < 0  -> влево
//  - TIME_SHIFT_X > 0  -> вправо
//
// ВАРИАНТЫ:
//  A) рекомендую: TIME_SHIFT_Y = -6
//  B) центр:      TIME_SHIFT_Y = 0
//  C) выше:       TIME_SHIFT_Y = -12
// =====================================================
static constexpr int TIME_SHIFT_X = 0;
static constexpr int TIME_SHIFT_Y = -6;   // ✅ Вариант A: чуть выше центра

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

    // Если экран сменился — запускаем fade (только HH:MM)
    uint32_t sv = uiVersion.version(UiChannel::SCREEN);
    if (sv != lastScreenV) {
        lastScreenV = sv;
        fadeActive = true;
        fadeStep = 0;
    }

    // Очищаем рабочую часть (ниже StatusBar) фоном темы
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
    // Важно: seconds всё равно будут обновляться через TIME channel,
    // поэтому мы специально синхронизируем lastTimeV внутри fade.
    if (fadeActive) {
        drawTime(true);

        // чтобы не пропустить реальное изменение времени пока идёт fade
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

    // ===== 1) Цвет HH:MM с fade =====
    // Fade применяется ТОЛЬКО к HH:MM, секунды не "тухнут".
    uint8_t a = 255;
    if (fadeActive) {
        uint16_t t = (uint16_t)fadeStep * 255 / FADE_STEPS;
        a = (t * t) / 255;
    }

    const uint16_t timeColor = fadeActive
        ? blend565(theme().bg, theme().textPrimary, a)
        : theme().textPrimary;

    // секунды — спокойные, вторичные
    const uint16_t secColor = theme().muted;

    tft.setFont(nullptr);
    tft.setTextWrap(false);

    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();

    // Ночью можно скрывать секунды (у тебя уже было так)
    const bool showSeconds = !night.isNight();

    // ===== 2) Геометрия надписи HH:MM =====
    // Это приблизительные метрики для setTextSize(3) и стандартного шрифта.
    const int DIGIT_W = 18;
    const int DIGIT_H = 24;
    const int TIME_W  = 5 * DIGIT_W; // "HH:MM" = 5 символов
    const int TIME_H  = DIGIT_H;

    // Safe-зона для часов (между линиями/панелями)
    const int safeY = layout.clockSafeY();
    const int safeH = layout.clockSafeH();

    // ===== 3) РАСПОЛОЖЕНИЕ HH:MM =====
    // Тут ты и хотел "поменять расположение":
    // мы считаем центр и добавляем сдвиги TIME_SHIFT_X/Y.
    const int X0 = (tft.width() - TIME_W) / 2;
    const int Y0 = safeY + (safeH - TIME_H) / 2;

    const int X = X0 + TIME_SHIFT_X;
    const int Y = Y0 + TIME_SHIFT_Y;

    // ===== 4) РАСПОЛОЖЕНИЕ секунд =====
    // Секунды ставим ПОД временем, под правым краем HH:MM.
    // Так они не "дерутся" за внимание и не расширяют строку вправо.
    const int SEC_W = 24;
    const int SEC_H = 12;

    const int SEC_X = X + TIME_W - SEC_W;   // под правым краем
    const int SEC_Y = Y + TIME_H + 4;       // чуть ниже

    // ===== 5) Очистка =====
    // force = true при смене темы / входе / fade-кадрах
    // тогда гарантированно очищаем зону HH:MM
    if (force) {
        tft.fillRect(X, Y, TIME_W, TIME_H, theme().bg);
    }

    // ===== 6) Рисуем HH:MM =====
    // Двоеточие мигает по версии TIME (как у тебя).
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

    // ===== 7) Рисуем секунды (локально) =====
    // Важно: мы очищаем ТОЛЬКО прямоугольник секунд,
    // чтобы весь экран не перемигивал.
    if (showSeconds) {
        tft.fillRect(SEC_X, SEC_Y, SEC_W, SEC_H, theme().bg);
        tft.setTextSize(1);
        tft.setTextColor(secColor, theme().bg);
        tft.setCursor(SEC_X, SEC_Y);
        tft.printf("%02d", s);
    } else if (force) {
        // если секунды скрыты — очищаем их область только при force,
        // чтобы "хвост" не оставался.
        tft.fillRect(SEC_X, SEC_Y, SEC_W, SEC_H, theme().bg);
    }
}