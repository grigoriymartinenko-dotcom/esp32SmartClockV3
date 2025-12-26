#pragma once
#include <Adafruit_ST7735.h>

/*
 * LayoutService
 * -------------
 * ЕДИНСТВЕННЫЙ источник геометрии UI.
 *
 * Зоны:
 *  - StatusBar (верх)
 *  - Header    (внутри content, сверху)
 *  - Content   (основная область)
 *  - BottomBar (низ)
 *
 * ПРАВИЛО:
 *  - НИКАКИХ магических чисел в экранах
 *  - ВСЕ координаты и отступы — отсюда
 */
class LayoutService {
public:
    explicit LayoutService(Adafruit_ST7735& tft);

    void begin();

    // ===== FLAGS =====
    void setHasStatusBar(bool v);
    void setHasBottomBar(bool v);

    bool hasStatusBar() const;
    bool hasBottomBar() const;

    // ===== HEIGHTS =====
    int statusBarH() const;
    int bottomBarH() const;
    int contentH() const;

    // ===== Y POSITIONS =====
    int statusBarY() const;
    int contentY() const;
    int bottomBarY() const;

    // ===== HEADER (inside content) =====
    int headerY() const;
    int headerHeight() const;
    int headerTextY() const;

    // ===== PADDING (ЕДИНЫЙ СТАНДАРТ) =====
    int padL() const;
    int padR() const;
    int padT() const;
    int padB() const;

    // ===== CONTENT BOUNDS =====
    int contentLeft() const;
    int contentRight() const;
    int contentW() const;

    // ===== CONTENT FLOW =====
    // Реальная стартовая Y-точка контента (ПОСЛЕ header)
    int contentTopY() const;
    int contentBottomY() const;

    // ===== TEXT METRICS =====
    int lineH() const;

    // ===== LEGACY ALIAS (ВРЕМЕННО) =====
    int statusY() const { return statusBarY(); }
    int statusH() const { return statusBarH(); }

    int buttonBarY() const { return bottomBarY(); }
    int buttonBarH() const { return bottomBarH(); }

    int bottomY() const { return bottomBarY(); }
    int bottomH() const { return bottomBarH(); }

    int clockSafeY() const { return contentY(); }
    int clockSafeH() const { return contentH(); }

    void setHasButtonBar(bool v) { setHasBottomBar(v); }

    int clockY() const { return contentY(); }

private:
    Adafruit_ST7735& _tft;

    bool _hasStatusBar = false;
    bool _hasBottomBar = false;

    // ===== FIXED UI METRICS =====
    static constexpr int STATUS_BAR_HEIGHT = 24;
    static constexpr int BOTTOM_BAR_HEIGHT = 26;

    static constexpr int HEADER_HEIGHT     = 20;
    static constexpr int HEADER_TEXT_PAD_Y = 4;

    // Padding — ЕДИНЫЙ ДЛЯ ВСЕГО UI
    static constexpr int PAD_L = 6;
    static constexpr int PAD_R = 6;
    static constexpr int PAD_T = 4;
    static constexpr int PAD_B = 4;

    // Высота строки (menu/list/text)
    static constexpr int LINE_HEIGHT = 14;
};