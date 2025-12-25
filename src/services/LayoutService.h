#pragma once
#include <Adafruit_ST7735.h>

/*
 * LayoutService
 * -------------
 * ЕДИНСТВЕННЫЙ источник геометрии UI.
 *
 * Зоны:
 *  - StatusBar (верх)
 *  - Content   (центр)
 *  - BottomBar (низ)
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

    // ===== LEGACY ALIAS (ВРЕМЕННО) =====

// status
int statusY() const { return statusBarY(); }
int statusH() const { return statusBarH(); }

// bottom / button
int buttonBarY() const { return bottomBarY(); }
int buttonBarH() const { return bottomBarH(); }

// very old names
int bottomY() const { return bottomBarY(); }
int bottomH() const { return bottomBarH(); }

// clock safe (legacy screens)
int clockSafeY() const { return contentY(); }
int clockSafeH() const { return contentH(); }

// flags
void setHasButtonBar(bool v) { setHasBottomBar(v); }

// ===== VERY OLD legacy (Forecast, etc.) =====
int clockY() const { return contentY(); }

private:
    Adafruit_ST7735& _tft;

    bool _hasStatusBar = false;
    bool _hasBottomBar = false;

    static constexpr int STATUS_BAR_HEIGHT = 24;
    static constexpr int BOTTOM_BAR_HEIGHT = 26; // единый стандарт
};