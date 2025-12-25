#pragma once
#include <Adafruit_ST7735.h>

/*
 * LayoutService
 * -------------
 * ЕДИНСТВЕННЫЙ источник координат UI.
 *
 * ЭТАП 1:
 *  - Введён новый контракт ButtonBar
 *  - Старый API сохранён как alias
 */
class LayoutService {
public:
    explicit LayoutService(Adafruit_ST7735& tft);

    void begin();

    // ===== FLAGS =====
    void setHasStatusBar(bool v);
    void setHasButtonBar(bool v);

    // legacy alias
    void setHasBottomBar(bool v) { setHasButtonBar(v); }

    bool hasStatusBar() const;
    bool hasButtonBar() const;

    // ===== NEW HEIGHTS =====
    int statusBarH() const;
    int buttonBarH() const;
    int contentH() const;

    // ===== NEW Y =====
    int statusBarY() const;
    int contentY() const;
    int buttonBarY() const;

    // ===== LEGACY API (ALIAS) =====
    int statusH() const { return statusBarH(); }
    int statusY() const { return statusBarY(); }

    int bottomH() const { return buttonBarH(); }
    int bottomY() const { return buttonBarY(); }

    int clockY() const { return contentY(); }

    // safe-zone для старых экранов
    int clockSafeY() const { return contentY(); }
    int clockSafeH() const { return contentH(); }

private:
    Adafruit_ST7735& _tft;

    bool _hasStatusBar = false;
    bool _hasButtonBar = false;

    static constexpr int STATUS_BAR_HEIGHT = 24;
    static constexpr int BUTTON_BAR_HEIGHT = 22;
};