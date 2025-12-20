#include "services/NightService.h"

NightService::NightService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void NightService::begin() {
    _mode = Mode::AUTO;
    _autoStartMin = 22 * 60;
    _autoEndMin   = 6 * 60;
    _isNight = false;
}

void NightService::setMode(Mode m) {
    if (_mode == m) return;

    _mode = m;
    _uiVersion.bump(UiChannel::THEME);
}

NightService::Mode NightService::mode() const {
    return _mode;
}

void NightService::setAutoRange(int startMin, int endMin) {
    if (startMin < 0) startMin = 0;
    if (startMin > 1439) startMin = 1439;
    if (endMin < 0) endMin = 0;
    if (endMin > 1439) endMin = 1439;

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

bool NightService::isNight() const {
    return _isNight;
}

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

bool NightService::computeAutoNight(const TimeService& time) const {

    if (!time.isValid())
        return false;

    const int nowMin = time.hour() * 60 + time.minute();

    // обычный интервал (например 20:00 → 23:00)
    if (_autoStartMin < _autoEndMin) {
        return (nowMin >= _autoStartMin) && (nowMin < _autoEndMin);
    }

    // интервал через полночь (22:00 → 06:00)
    return (nowMin >= _autoStartMin) || (nowMin < _autoEndMin);
}