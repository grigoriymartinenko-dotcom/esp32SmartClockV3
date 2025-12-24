#include "screens/SettingsScreen.h"

/*
 * SettingsEdit.cpp
 * ----------------
 * Вся логика изменения значений (EDIT режим).
 *
 * ПРАВИЛА:
 *  - < >  → editDec / editInc
 *  - OK   → вход в EDIT
 *  - OK+  → APPLY
 *  - BACK → CANCEL
 */

// ============================================================================
// ENTER EDIT
// ============================================================================
void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;

    // backup значений
    if (_level == Level::WIFI) {
        _bakWifiOn = _tmpWifiOn;
    }

    if (_level == Level::TIME) {
        _bakTimeMode = _tmpTimeMode;
    }

    if (_level == Level::NIGHT) {
        _bakMode       = _tmpMode;
        _bakNightStart = _tmpNightStart;
        _bakNightEnd   = _tmpNightEnd;
    }

    if (_level == Level::TIMEZONE) {
        _bakTzSec  = _tmpTzSec;
        _bakDstSec = _tmpDstSec;
    }

    _dirty = true;
}

// ============================================================================
// EXIT EDIT
// ============================================================================
void SettingsScreen::exitEdit(bool apply) {

    if (!apply) {
        // rollback
        if (_level == Level::WIFI) {
            _tmpWifiOn = _bakWifiOn;
        }

        if (_level == Level::TIME) {
            _tmpTimeMode = _bakTimeMode;
        }

        if (_level == Level::NIGHT) {
            _tmpMode       = _bakMode;
            _tmpNightStart = _bakNightStart;
            _tmpNightEnd   = _bakNightEnd;
        }

        if (_level == Level::TIMEZONE) {
            _tmpTzSec  = _bakTzSec;
            _tmpDstSec = _bakDstSec;
        }
    }

    // APPLY делаем НЕ здесь
    _mode = UiMode::NAV;
    _dirty = true;
}

// ============================================================================
// EDIT INC
// ============================================================================
void SettingsScreen::editInc() {

    // ------------------------------------------------------------
    // WIFI
    // ------------------------------------------------------------
    if (_level == Level::WIFI && _subSelected == 0) {
        _tmpWifiOn = !_tmpWifiOn;
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // TIME
    // ------------------------------------------------------------
    if (_level == Level::TIME && _subSelected == 0) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 1) % 4);
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // NIGHT
    // ------------------------------------------------------------
    if (_level == Level::NIGHT) {
        if (_subSelected == 0) {
            int v = (int)_tmpMode;
            _tmpMode = (NightService::Mode)((v + 1) % 3);
        } else if (_subSelected == 1) {
            _tmpNightStart = (_tmpNightStart + NIGHT_STEP_MIN) % 1440;
        } else if (_subSelected == 2) {
            _tmpNightEnd = (_tmpNightEnd + NIGHT_STEP_MIN) % 1440;
        }
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // TIMEZONE
    // ------------------------------------------------------------
    if (_level == Level::TIMEZONE) {
        if (_subSelected == 0) {
            _tmpTzSec = min(_tmpTzSec + TZ_STEP, TZ_MAX);
        } else if (_subSelected == 1) {
            _tmpDstSec = min(_tmpDstSec + TZ_STEP, TZ_MAX);
        }
        _dirty = true;
        return;
    }
}

// ============================================================================
// EDIT DEC
// ============================================================================
void SettingsScreen::editDec() {

    // ------------------------------------------------------------
    // WIFI
    // ------------------------------------------------------------
    if (_level == Level::WIFI && _subSelected == 0) {
        _tmpWifiOn = !_tmpWifiOn;
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // TIME
    // ------------------------------------------------------------
    if (_level == Level::TIME && _subSelected == 0) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 3) % 4);
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // NIGHT
    // ------------------------------------------------------------
    if (_level == Level::NIGHT) {
        if (_subSelected == 0) {
            int v = (int)_tmpMode;
            _tmpMode = (NightService::Mode)((v + 2) % 3);
        } else if (_subSelected == 1) {
            _tmpNightStart = (_tmpNightStart + 1440 - NIGHT_STEP_MIN) % 1440;
        } else if (_subSelected == 2) {
            _tmpNightEnd = (_tmpNightEnd + 1440 - NIGHT_STEP_MIN) % 1440;
        }
        _dirty = true;
        return;
    }

    // ------------------------------------------------------------
    // TIMEZONE
    // ------------------------------------------------------------
    if (_level == Level::TIMEZONE) {
        if (_subSelected == 0) {
            _tmpTzSec = max(_tmpTzSec - TZ_STEP, TZ_MIN);
        } else if (_subSelected == 1) {
            _tmpDstSec = max(_tmpDstSec - TZ_STEP, TZ_MIN);
        }
        _dirty = true;
        return;
    }
} 