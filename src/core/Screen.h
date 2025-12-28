#pragma once

#include <stdint.h>
#include "services/ThemeService.h"

/*
 * Screen
 * ------
 * Базовый класс для всех экранов UI.
 *
 * ЦЕЛИ:
 *  - сохранить совместимость со старым UI
 *  - добавить корректный доступ к ThemeService для ThemeBlend-only экранов
 *
 * ПРАВИЛА:
 *  - Старый UI использует theme() -> Theme
 *  - Новый UI использует themeService().blend() -> ThemeBlend
 */

class Screen {
public:
    explicit Screen(ThemeService& themeService)
        : _themeService(themeService)
    {}

    virtual ~Screen() = default;

    // =====================================================================
    // Lifecycle
    // =====================================================================
    virtual void begin() {}
    virtual void update() {}

    // =====================================================================
    // UI flags
    // =====================================================================
    // Исторически используется в проекте (Settings / ScreenManager)
    virtual bool hasButtonBar() const { return true; }

    // Более новое имя (можно использовать параллельно)
    virtual bool hasBottomBar() const { return hasButtonBar(); }

    virtual bool hasStatusBar() const { return true; }

    // =====================================================================
    // Theme hooks
    // =====================================================================
    // Используется SettingsScreen
    virtual void onThemeChanged() {}

protected:
    // =====================================================================
    // OLD API (НЕ ТРОГАТЬ — используется старым UI)
    // =====================================================================
    const Theme& theme() const {
        return _themeService.current();
    }

    // =====================================================================
    // NEW API (для ThemeBlend-only экранов)
    // =====================================================================
    ThemeService& themeService() {
        return _themeService;
    }

    const ThemeService& themeService() const {
        return _themeService;
    }

private:
    ThemeService& _themeService;
};