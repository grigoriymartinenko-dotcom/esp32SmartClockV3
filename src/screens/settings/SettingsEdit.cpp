#include "screens/SettingsScreen.h"
#include <Arduino.h>

// ============================================================================
// EDIT CORE
// ============================================================================
void SettingsScreen::enterEdit() {
    _mode = UiMode::EDIT;

    if (_level == Level::TIME) {
        _bakTimeMode = _tmpTimeMode;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        _bakTzSec  = _tmpTzSec;
        _bakDstSec = _tmpDstSec;
        _dirty = true;
        return;
    }

    if (_level == Level::NIGHT) {
        _bakMode       = _tmpMode;
        _bakNightStart = _tmpNightStart;
        _bakNightEnd   = _tmpNightEnd;
        _dirty = true;
        return;
    }

    _dirty = true;
}

void SettingsScreen::exitEdit(bool apply) {
    if (!apply) {
        if (_level == Level::TIME) {
            _tmpTimeMode = _bakTimeMode;
        }

        if (_level == Level::TIMEZONE) {
            _tmpTzSec  = _bakTzSec;
            _tmpDstSec = _bakDstSec;
        }

        if (_level == Level::NIGHT) {
            _tmpMode       = _bakMode;
            _tmpNightStart = _bakNightStart;
            _tmpNightEnd   = _bakNightEnd;
        }

        _mode = UiMode::NAV;
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        extern PreferencesService prefs;
        prefs.setTimezone(_tmpTzSec, _tmpDstSec);
        prefs.save();
        _time.setTimezone((long)_tmpTzSec, (int)_tmpDstSec);
    }

    _mode = UiMode::NAV;
    _dirty = true;
}

// ============================================================================
// EDIT ACTIONS
// ============================================================================
void SettingsScreen::editInc() {
    if (_level == Level::TIME) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 1) % 4);
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        if (_subSelected == 0) {
            _tmpTzSec = constrain(_tmpTzSec + TZ_STEP, TZ_MIN, TZ_MAX);
        } else if (_subSelected == 1) {
            _tmpDstSec = constrain(_tmpDstSec + TZ_STEP, TZ_MIN, TZ_MAX);
        }
        _dirty = true;
        return;
    }

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
    }
}

void SettingsScreen::editDec() {
    if (_level == Level::TIME) {
        int v = (int)_tmpTimeMode;
        _tmpTimeMode = (TimeService::Mode)((v + 3) % 4);
        _dirty = true;
        return;
    }

    if (_level == Level::TIMEZONE) {
        if (_subSelected == 0) {
            _tmpTzSec = constrain(_tmpTzSec - TZ_STEP, TZ_MIN, TZ_MAX);
        } else if (_subSelected == 1) {
            _tmpDstSec = constrain(_tmpDstSec - TZ_STEP, TZ_MIN, TZ_MAX);
        }
        _dirty = true;
        return;
    }

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
    }
}