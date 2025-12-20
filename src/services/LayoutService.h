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

    // ===== BUTTON BAR (Settings / internal) =====
    int buttonBarH() const;
    int buttonBarY() const;

    // ===== SEPARATORS (ЛОГИЧЕСКАЯ ПОЗИЦИЯ) =====
    int sepStatusY() const;   // линия СРАЗУ под StatusBar
    int sepBottomY() const;   // линия СРАЗУ над BottomBar

    // ===== SAFE CLOCK RECT =====
    int clockSafeY() const;
    int clockSafeH() const;

private:
    Adafruit_ST7735& _tft;

    static constexpr int STATUS_H = 24;

    // BottomBar (датчики) — как было
    static constexpr int BOTTOM_H = 36;

    // ButtonBar (кнопки) — ниже на ~5мм (меньше высота => визуально "ниже")
    // 5мм ≈ 18–20px → берём 20px.
    static constexpr int BUTTONBAR_H = 20;

    bool _hasBottomBar = true;
};