#include "services/ThemeService.h"
#include "theme/Themes.h"

ThemeService::ThemeService(UiVersionService& uiVersion)
    : _uiVersion(uiVersion)
{
}

void ThemeService::begin() {
    _theme = THEME_DAY;
    _night = false;
}

void ThemeService::setNight(bool night) {
    if (_night == night)
        return;

    _night = night;
    _theme = _night ? THEME_NIGHT : THEME_DAY;

    // 🔥 логическое событие
    _uiVersion.bump(UiChannel::THEME);
}

bool ThemeService::isNight() const {
    return _night;
}

const Theme& ThemeService::current() const {
    return _theme;
}