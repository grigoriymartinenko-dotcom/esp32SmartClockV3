#include "services/NightService.h"

NightService::NightService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void NightService::begin() {
    _isNight = false;
}

void NightService::update(const TimeService& time) {
    bool night =
        (time.hour() >= 22) ||
        (time.hour() < 6);

    if (night != _isNight) {
        _isNight = night;

        // 🔹 логическое событие: сменилась ночь/день
        _uiVersion.bump(UiChannel::THEME);
    }
}

bool NightService::isNight() const {
    return _isNight;
}