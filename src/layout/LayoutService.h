#pragma once
#include <Adafruit_ST7735.h>

/*
 * LayoutService
 * -------------
 * Единственный источник координат и размеров UI-зон.
 *
 * Правило:
 *  - экраны (ClockScreen) рисуют ТОЛЬКО внутри safe-областей
 *  - линии/сепараторы рисуются отдельно (UiSeparator)
 */
class LayoutService {
public:
    explicit LayoutService(Adafruit_ST7735& tft);

    void begin();

    // ===== HEIGHTS =====
    int statusH() const;
    int clockH()  const;
    int bottomH() const;

    // ===== Y POSITIONS =====
    int statusY() const;
    int clockY()  const;
    int bottomY() const;

    // ===== SEPARATORS (визуальные линии) =====
    int sepStatusY() const;   // линия после StatusBar
    int sepBottomY() const;   // линия перед BottomBar

    // ===== SAFE CLOCK RECT (область, где ClockScreen МОЖЕТ рисовать) =====
    int clockSafeY() const;   // старт ниже верхней линии
    int clockSafeH() const;   // высота до нижней линии

private:
    Adafruit_ST7735& _tft;

    static constexpr int STATUS_H = 24;
    static constexpr int BOTTOM_H = 36;

    // визуальные отступы линий от зон
    static constexpr int SEP_OFFSET = 2;

    int _clockH = 0;
};