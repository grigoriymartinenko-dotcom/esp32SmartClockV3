#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"

/*
 * SettingsScreen
 * --------------
 * Полноэкранные настройки.
 *
 * Правила:
 *  - hasStatusBar() = false
 *  - hasBottomBar() = false (BottomBar скрываем)
 *  - ButtonBar рисуем ВНУТРИ SettingsScreen
 *  - Без millis()/таймеров: всё реактивно по событиям (кнопки/смена темы)
 *
 * ВАЖНО:
 *  - Переключение экранов делает main/ScreenManager.
 *  - SettingsScreen только выставляет флаг exitRequested().
 */

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return false; }

    void onThemeChanged() override;

    // ===== input events from main =====
    void onLeft();
    void onRight();
    void onOk();
    void onBack();

    bool exitRequested() const;
    void clearExitRequest();

private:
    void redrawAll();
    void drawTitle();
    void drawList();

private:
    Adafruit_ST7735& _tft;
    LayoutService&   _layout;

    ButtonBar _bar;

    bool _dirty = true;
    int  _selected = 0;

    // демо-настройка (следующим шагом заменим на NightModeService)
    bool _nightAuto = true;

    bool _exitRequested = false;

    static constexpr int ITEM_COUNT = 4;
};