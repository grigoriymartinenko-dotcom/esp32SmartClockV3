#pragma once
#include <Adafruit_ST7735.h>

/*
 * LayoutService
 * -------------
 * ЕДИНСТВЕННЫЙ источник координат UI.
 *
 * ПРИНЦИП:
 *  - LayoutService оперирует ЛОГИЧЕСКИМИ зонами
 *  - Толщина линий = ответственность UiSeparator
 *  - НИКАКИХ offset / магических +1 / -1
 */
class LayoutService {
public:
    explicit LayoutService(Adafruit_ST7735& tft);

    void begin();

    // ===== FLAGS =====
    void setHasBottomBar(bool v);
    bool hasBottomBar() const;

    // ===== HEIGHTS =====
    int statusH() const;
    int clockH()  const;
    int bottomH() const;

    // ===== Y POSITIONS =====
    int statusY() const;
    int clockY()  const;
    int bottomY() const;

    // ===== SEPARATORS (ЛОГИЧЕСКАЯ ПОЗИЦИЯ) =====
    int sepStatusY() const;   // линия СРАЗУ под StatusBar
    int sepBottomY() const;   // линия СРАЗУ над BottomBar

    // ===== SAFE CLOCK RECT =====
    int clockSafeY() const;
    int clockSafeH() const;

private:
    Adafruit_ST7735& _tft;

    static constexpr int STATUS_H = 24;
    static constexpr int BOTTOM_H = 36;

    bool _hasBottomBar = true;
};