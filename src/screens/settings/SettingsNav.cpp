#include "screens/SettingsScreen.h"

// ============================================================================
// submenu items count
// ============================================================================
int SettingsScreen::submenuItemsCount() const {
    switch (_level) {
        case Level::TIMEZONE: return 2; // UTC, DST
        case Level::TIME:     return 1; // Source
        case Level::NIGHT:    return 3; // Mode, Start, End
        case Level::ROOT:     return (int)(sizeof(MENU) / sizeof(MENU[0]));
    }
    return 1;
}

// ============================================================================
// NAVIGATION
// ============================================================================
void SettingsScreen::navLeft() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + n - 1) % n;
        _dirty = true;
        return;
    }

    int n = submenuItemsCount();
    if (n <= 1) return;

    _subSelected = (_subSelected + n - 1) % n;
    _dirty = true;
}

void SettingsScreen::navRight() {
    if (_level == Level::ROOT) {
        int n = sizeof(MENU) / sizeof(MENU[0]);
        _selected = (_selected + 1) % n;
        _dirty = true;
        return;
    }

    int n = submenuItemsCount();
    if (n <= 1) return;

    _subSelected = (_subSelected + 1) % n;
    _dirty = true;
}