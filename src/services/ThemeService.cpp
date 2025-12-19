#include "services/ThemeService.h"
#include "theme/Themes.h"

void ThemeService::begin() {
    _isNight = false;
}

bool ThemeService::setNight(bool night) {
    if (_isNight == night)
        return false;

    _isNight = night;
    return true;
}

bool ThemeService::isNight() const {
    return _isNight;
}

const Theme& ThemeService::current() const {
    // ------------------------------------------------------------
    // ВАЖНО:
    // Темы определены как constexpr (THEME_DAY / THEME_NIGHT),
    // поэтому возвращаем ССЫЛКУ на них.
    // ------------------------------------------------------------
    return _isNight ? THEME_NIGHT : THEME_DAY;
}