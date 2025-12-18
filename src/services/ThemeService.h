#pragma once
#include "theme/Themes.h"

class ThemeService {
public:
    void begin();
    void update();
    const Theme& current() const;
    void setNight(bool night);

private:
    const Theme* _current = &THEME_DAY;
};
