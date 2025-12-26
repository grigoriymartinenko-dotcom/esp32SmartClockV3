#pragma once
#include "services/ThemeService.h"

/*
 * Screen
 * ------
 * Базовый интерфейс экрана.
 *
 * ПРАВИЛА:
 *  - Screen НИЧЕГО не знает про реализацию ButtonBar
 *  - ScreenManager решает геометрию через LayoutService
 *
 * ВАЖНО:
 *  - По умолчанию нижняя панель аппаратных кнопок ВКЛЮЧЕНА (hasButtonBar=true),
 *    потому что это базовый UX устройства.
 *  - Экран, которому она не нужна, ЯВНО делает override и возвращает false.
 *
 * LEGACY:
 *  - hasBottomBar() оставлен для совместимости, но является алиасом hasButtonBar()
 */

class Screen {
public:
    virtual ~Screen() = default;

    // lifecycle
    virtual void begin() = 0;
    virtual void update() = 0;

    // есть ли статусбар на этом экране
    virtual bool hasStatusBar() const { return true; }

    // ✅ БАЗОВО: нижняя панель кнопок включена
    virtual bool hasButtonBar() const { return true; }

    // legacy alias
    virtual bool hasBottomBar() const { return hasButtonBar(); }

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