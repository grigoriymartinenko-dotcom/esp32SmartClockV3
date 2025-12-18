#include "services/ThemeService.h"

void ThemeService::begin() {
    _current = &THEME_DAY;
}

void ThemeService::update() {}

const Theme& ThemeService::current() const {
    return *_current;
}

void ThemeService::setNight(bool night) {
    _current = night ? &THEME_NIGHT : &THEME_DAY;
}
