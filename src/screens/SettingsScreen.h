#pragma once
#include <Adafruit_ST7735.h>

#include "core/Screen.h"
#include "services/LayoutService.h"
#include "services/NightService.h"
#include "services/UiVersionService.h"
#include "ui/ButtonBar.h"

/*
 * SettingsScreen
 * --------------
 * Полноэкранные настройки.
 *
 * Правила:
 *  - hasStatusBar() = false
 *  - hasBottomBar() = false
 *  - ButtonBar рисуем ВНУТРИ SettingsScreen
 *  - Без millis()/таймеров: всё реактивно (кнопки/версии)
 *
 * Night UX:
 *  - Night mode: AUTO / ON / OFF
 *  - В AUTO: редактируем Night start / Night end (HH:MM)
 *  - Мигают HH/MM (через UiChannel::TIME)
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

    // ===== input events from main =====
    void onLeft();
    void onRight();
    void onOk();
    void onBack();

    bool exitRequested() const;
    void clearExitRequest();

private:
    enum Item : uint8_t {
        ITEM_NIGHT_MODE = 0,
        ITEM_NIGHT_START,
        ITEM_NIGHT_END,
        ITEM_ABOUT,
        ITEM_COUNT
    };

    enum class EditState : uint8_t {
        NONE = 0,
        EDIT_START,
        EDIT_END
    };

    enum class EditField : uint8_t {
        HOURS = 0,
        MINUTES
    };

private:
    void redrawAll();
    void drawTitle();
    void drawList();

    // helpers
    static void formatHHMM(char* out, int minutes);
    static int  clampMin(int v);
    static int  addMinutes(int v, int delta);

    void cycleNightMode();
    bool isAuto() const;

    // edit flow
    void startEdit(EditState s);
    void applyEditDelta(int delta);      // +/- on selected field
    void toggleEditField();              // OK inside edit
    void commitEditAndExit();            // BACK inside edit

private:
    Adafruit_ST7735& _tft;
    LayoutService&   _layout;

    NightService&    _night;
    UiVersionService& _uiVersion;

    ButtonBar _bar;

    bool _dirty = true;
    bool _exitRequested = false;

    int  _selected = 0;

    // UI state
    EditState _editState = EditState::NONE;
    EditField _editField = EditField::HOURS;

    // временные значения при редактировании (минуты от 00:00)
    int _tmpStartMin = 22 * 60;
    int _tmpEndMin   = 6 * 60;

    // чтобы лишний раз не перерисовывать без нужды
    NightService::Mode _lastMode = NightService::Mode::AUTO;
    int _lastStartMin = -1;
    int _lastEndMin   = -1;
    uint32_t _lastBlinkV = 0;
};