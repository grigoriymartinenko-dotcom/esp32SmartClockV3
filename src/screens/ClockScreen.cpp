#include "screens/ClockScreen.h"
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "ui/Layout.h"
/*
 * ============================================================
 * CLOCK SCREEN — ОСНОВНОЙ ЭКРАН ЧАСОВ
 *
 * Архитектура:
 *  - HH и MM перерисовываются ТОЛЬКО при смене минут
 *  - двоеточие мигает КАЖДУЮ секунду
 *  - секунды рисуются как индекс справа от минут
 *  - никаких лишних перерисовок (бережём TFT)
 * ============================================================
 */

// ширина экрана ST7735 (160x128)
static constexpr int SCREEN_W = 160;

// -------------------- LAYOUT --------------------

// координаты горизонтальных линий
static constexpr int TOP_LINE_Y    = 22;
static constexpr int BOTTOM_LINE_Y = 78;

// baseline времени HH:MM
// (baseline — это НЕ верх текста, а линия, на которой стоят символы)
static constexpr int TIME_BASE_Y   = 62;

// -------------------- SPACING --------------------

// расстояние между HH и :
static constexpr int COLON_GAP = 6;

// юстировка двоеточия относительно baseline HH/MM
// у шрифта двоеточие визуально "завалено", поэтому правим вручную
static constexpr int COLON_X_OFFSET = -2;  // влево
static constexpr int COLON_Y_OFFSET = -5;  // вверх

// секунды как индекс (справа от минут)
static constexpr int SEC_X_GAP    = 49;    // отступ от MM вправо
static constexpr int SEC_Y_OFFSET = -12;   // выше baseline минут

// ------------------------------------------------

ClockScreen::ClockScreen(
    Adafruit_ST7735& t,
    TimeService& ts,
    NightService& ns,
    ThemeService& themeSvc
)
: Screen(themeSvc),   // базовый Screen хранит ссылку на ThemeService
  tft(t),
  time(ts),
  night(ns)
{}

void ClockScreen::begin() {
    const Theme& th = theme();

    // ------------------------------------------------------------
    // begin() = ПОЛНЫЙ reset визуального состояния экрана
    // ------------------------------------------------------------

    lastH = lastM = lastS = -1;
    lastSecShown  = -1;

    lastTempShown = -10000;
    lastHumShown  = -1;

    // Чистим ТОЛЬКО область ниже статусбара
    tft.fillRect(
        0,
        STATUS_BAR_H,
        SCREEN_W,
        tft.height() - STATUS_BAR_H,
        th.bg
    );

    // Разделительные линии
    tft.drawFastHLine(0, TOP_LINE_Y, SCREEN_W, th.accent);
    tft.drawFastHLine(0, BOTTOM_LINE_Y, SCREEN_W, th.accent);
}

void ClockScreen::update() {
    if (!time.isValid()) return;

    // ------------------------------------------------------------
    // DHT11 — обновляем РЕДКО
    // ------------------------------------------------------------
    // Экран сам решает, когда читать датчик,
    // чтобы не дергать его каждый кадр.
    //
    // Интервал > 3 сек:
    //  - безопасно для DHT11
    //  - достаточно для UI
    // ------------------------------------------------------------
    uint32_t now = millis();
    if (now - lastDhtUpdateMs >= 3000) {
        dht.update();
        lastDhtUpdateMs = now;
    }

    drawTime();
    drawDht();
}

void ClockScreen::drawTime() {
    const Theme& th = theme();

    int h = time.hour();
    int m = time.minute();
    int s = time.second();

    /*
     * ============================================================
     * HH и MM
     * ------------------------------------------------------------
     * Перерисовываются ТОЛЬКО при смене минут или часов.
     * Это:
     *  - устраняет мигание
     *  - экономит SPI / TFT
     * ============================================================
     */
    if (h != lastH || m != lastM) {

        // очищаем ТОЛЬКО область между линиями
        tft.fillRect(
            0,
            TOP_LINE_Y + 2,
            SCREEN_W,
            BOTTOM_LINE_Y - TOP_LINE_Y - 3,
            th.bg
        );

        // строка для вычисления центрирования
        char full[6];
        snprintf(full, sizeof(full), "%02d:%02d", h, m);

        // считаем ширину "HH:MM"
        int16_t x1, y1;
        uint16_t wFull, hgt;
        tft.setFont(&FreeSansBold18pt7b);
        tft.getTextBounds(full, 0, 0, &x1, &y1, &wFull, &hgt);

        // старт X так, чтобы вся строка была по центру экрана
        int xStart = (SCREEN_W - (int)wFull) / 2;

        tft.setTextColor(th.primary, th.bg);

        // ---------- HH ----------
        char hh[3];
        snprintf(hh, sizeof(hh), "%02d", h);

        uint16_t wHH;
        tft.getTextBounds(hh, 0, 0, &x1, &y1, &wHH, &hgt);

        tft.setCursor(xStart, TIME_BASE_Y);
        tft.print(hh);

        // ---------- MM ----------
        char mm[3];
        snprintf(mm, sizeof(mm), "%02d", m);

        uint16_t wColon;
        tft.getTextBounds(":", 0, 0, &x1, &y1, &wColon, &hgt);

        int mmX = xStart + wHH + COLON_GAP + wColon + COLON_GAP;
        tft.setCursor(mmX, TIME_BASE_Y);
        tft.print(mm);

        lastH = h;
        lastM = m;
    }

    /*
     * ============================================================
     * МИГАНИЕ ДВОЕТОЧИЯ
     * ------------------------------------------------------------
     * Обновляется КАЖДУЮ секунду.
     * Рисуется только на чётных секундах.
     * ============================================================
     */
    if (s != lastS) {

        tft.setFont(&FreeSansBold18pt7b);

        // пересчитываем центр (без перерисовки HH/MM)
        char full[6];
        snprintf(full, sizeof(full), "%02d:%02d", h, m);

        int16_t x1, y1;
        uint16_t wFull, hgt;
        tft.getTextBounds(full, 0, 0, &x1, &y1, &wFull, &hgt);

        int xStart = (SCREEN_W - (int)wFull) / 2;

        uint16_t wHH;
        tft.getTextBounds("00", 0, 0, &x1, &y1, &wHH, &hgt);

        // позиция двоеточия (с ручной юстировкой)
        int colonX = xStart + wHH + COLON_GAP + COLON_X_OFFSET;
        int colonY = TIME_BASE_Y + COLON_Y_OFFSET;

        // очищаем область двоеточия с запасом
        tft.fillRect(
            colonX - 4,
            colonY - 22,
            12,
            28,
            th.bg
        );

        // рисуем двоеточие только на чётных секундах
        if ((s & 1) == 0) {
            tft.setCursor(colonX, colonY);
            tft.setTextColor(th.primary, th.bg);
            tft.print(":");
        }

        lastS = s;
    }

    /*
     * ============================================================
     * СЕКУНДЫ КАК ИНДЕКС
     * ------------------------------------------------------------
     * Маленький шрифт, справа от минут, без мигания.
     * ============================================================
     */
/* ========= СЕКУНДЫ КАК ИНДЕКС ========= */
/* =========================================================
 * СЕКУНДЫ КАК ИНДЕКС
 * ---------------------------------------------------------
 * Обновляются ТОЛЬКО если значение секунд изменилось
 * ========================================================= */
if (s != lastSecShown) {

    tft.setFont(&FreeSans9pt7b);

    int16_t x1, y1;
    uint16_t wSec, hSec;
    tft.getTextBounds("88", 0, 0, &x1, &y1, &wSec, &hSec);

    char ss[3];
    snprintf(ss, sizeof(ss), "%02d", s);

    int secX = SCREEN_W / 2 + SEC_X_GAP;
    int secY = TIME_BASE_Y + SEC_Y_OFFSET;

    // чистим ТОЛЬКО область секунд
    tft.fillRect(
        secX - 1,
        secY - hSec,
        wSec + 2,
        hSec + 2,
        th.bg
    );

    tft.setTextColor(th.secondary, th.bg);
    tft.setCursor(secX, secY);
    tft.print(ss);

    lastSecShown = s;
}

    // линии рисуем в конце, чтобы они всегда были поверх
    tft.drawFastHLine(0, TOP_LINE_Y, SCREEN_W, th.accent);
    tft.drawFastHLine(0, BOTTOM_LINE_Y, SCREEN_W, th.accent);
}

void ClockScreen::drawDht() {
    // 1. Если данных ещё нет — вообще ничего не делаем
    if (!dht.valid()) return;

    // 2. Приводим данные к целым значениям для сравнения
    //    (температура *10 → учитываем 0.1°C)
    int temp10 = (int)(dht.temperature() * 10.0f);
    int hum    = (int)(dht.humidity());

    // 3. Если значения НЕ ИЗМЕНИЛИСЬ —
    //    НЕ чистим экран, НЕ рисуем, НЕ трогаем SPI
    if (temp10 == lastTempShown && hum == lastHumShown) {
        return;
    }

    // 4. Запоминаем, что сейчас будем рисовать
    lastTempShown = temp10;
    lastHumShown  = hum;

    const Theme& th = theme();

    // 5. Координаты нижней панели
    const int y   = BOTTOM_LINE_Y + 6;
    const int hgt = 18;

    // 6. ЧИСТИМ ТОЛЬКО ЭТУ ЗОНУ
    tft.fillRect(0, y, SCREEN_W, hgt, th.bg);

    // 7. Рисуем текст
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(th.secondary, th.bg);
    tft.setCursor(6, y + 14);

    tft.printf(
        "IN %.1fC  %d%%",
        dht.temperature(),
        hum
    );
}