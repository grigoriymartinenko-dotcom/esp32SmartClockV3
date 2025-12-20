#pragma once
#include "services/ThemeService.h"

class Screen {
public:
    virtual ~Screen() = default;

    virtual void begin() = 0;
    virtual void update() = 0;

    // 🔹 есть ли статусбар на этом экране
    virtual bool hasStatusBar() const { return true; }

    // 🔹 есть ли нижний бар на этом экране
    virtual bool hasBottomBar() const { return true; }

    // хук смены темы
    virtual void onThemeChanged() {}

protected:
    ThemeService& themeService;

    explicit Screen(ThemeService& ts)
        : themeService(ts) {}

    const Theme& theme() const {
        return themeService.current();
    }
};