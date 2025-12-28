#include "services/NightService.h"

// ============================================================================
// ctor
// ============================================================================
NightService::NightService(
    UiVersionService& uiVersion,
    PreferencesService& prefs
)
    : _uiVersion(uiVersion)
    , _prefs(prefs)
{
}

// ============================================================================
// begin — ВАЖНО!
// ============================================================================
void NightService::begin() {

    // ===== LOAD FROM EEPROM =====
    _mode = static_cast<Mode>(_prefs.nightMode());

    _autoStartMin = _prefs.nightStart();
    _autoEndMin   = _prefs.nightEnd();

    // защита от мусора
    if (_autoStartMin < 0 || _autoStartMin > 1439)
        _autoStartMin = 22 * 60;

    if (_autoEndMin < 0 || _autoEndMin > 1439)
        _autoEndMin = 6 * 60;

    _isNight = false;

    // логическое событие
    _uiVersion.bump(UiChannel::THEME);
}

// ============================================================================
// MODE
// ============================================================================
void NightService::setMode(Mode m) {
    if (_mode == m)
        return;

    _mode = m;
    _uiVersion.bump(UiChannel::THEME);
}

NightService::Mode NightService::mode() const {
    return _mode;
}

// ============================================================================
// AUTO RANGE
// ============================================================================
void NightService::setAutoRange(int startMin, int endMin) {

    startMin = constrain(startMin, 0, 1439);
    endMin   = constrain(endMin,   0, 1439);

    if (_autoStartMin == startMin && _autoEndMin == endMin)
        return;

    _autoStartMin = startMin;
    _autoEndMin   = endMin;

    if (_mode == Mode::AUTO) {
        _uiVersion.bump(UiChannel::THEME);
    }
}

int NightService::autoStart() const {
    return _autoStartMin;
}

int NightService::autoEnd() const {
    return _autoEndMin;
}

// ============================================================================
// RESULT
// ============================================================================
bool NightService::isNight() const {
    return _isNight;
}

// ============================================================================
// UPDATE
// ============================================================================
void NightService::update(const TimeService& time) {

    bool night = false;

    switch (_mode) {
        case Mode::ON:
            night = true;
            break;

        case Mode::OFF:
            night = false;
            break;

        case Mode::AUTO:
            night = computeAutoNight(time);
            break;
    }

    if (night != _isNight) {
        _isNight = night;
        _uiVersion.bump(UiChannel::THEME);
    }
}

// ============================================================================
// AUTO LOGIC
// ============================================================================
bool NightService::computeAutoNight(const TimeService& time) const {

    if (!time.isValid())
        return false;

    const int nowMin = time.hour() * 60 + time.minute();

    // обычный интервал (например 20:00 → 23:00)
    if (_autoStartMin < _autoEndMin) {
        return (nowMin >= _autoStartMin) && (nowMin < _autoEndMin);
    }

    // через полночь (22:00 → 06:00)
    return (nowMin >= _autoStartMin) || (nowMin < _autoEndMin);
}