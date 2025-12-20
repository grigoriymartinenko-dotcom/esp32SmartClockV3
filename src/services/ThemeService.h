#pragma once
#include <stdint.h>

#include "services/UiVersionService.h"
#include "theme/Themes.h"

/*
 * ThemeService
 * ------------
 * Управляет текущей темой (day / night).
 * Любая смена темы → bump UiChannel::THEME
 */

class ThemeService {
public:
    explicit ThemeService(UiVersionService& uiVersion);

    void begin();

    void setNight(bool night);
    bool isNight() const;

    const Theme& current() const;

private:
    UiVersionService& _uiVersion;

    bool  _night = false;
    Theme _theme;
};