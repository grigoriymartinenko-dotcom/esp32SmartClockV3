#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "ui/ButtonBar.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"

/*
 * SettingsScreen (step: navigation skeleton)
 * -----------------------------------------
 * Сейчас:
 *  - корневое меню (список)
 *  - flash кнопок
 *
 * Добавили:
 *  - onOkLong()  : вход в подменю (заготовка)
 *  - onBackLong(): выход назад/из экрана (классический UX)
 *
 * Следующий шаг:
 *  - реальные подменю Night Mode / Timezone
 */

class SettingsScreen : public Screen {
public:
    SettingsScreen(
        Adafruit_ST7735& tft,
        ThemeService& themeService,
        LayoutService& layoutService,
        NightService& nightService,
        UiVersionService& uiVersion
    );

    void begin() override;
    void update() override;

    bool hasStatusBar() const override { return false; }
    bool hasBottomBar() const override { return false; }

    void onThemeChanged() override;

    // short press
    void onLeft();
    void onRight();
    void onOk();
    void onBack();

    // long press
    void onOkLong();
    void onBackLong();

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

    NightService& _night;
    UiVersionService& _ui;

    bool _dirty = true;
    int  _selected = 0;

    bool _exitRequested = false;

    static constexpr int ITEM_COUNT = 6; // расширили на "пустышки"
};