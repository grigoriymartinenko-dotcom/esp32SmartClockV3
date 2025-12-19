#pragma once
#include "theme/Theme.h"

/*
 * ThemeService
 * ------------
 * Хранит текущее состояние темы (день / ночь)
 * и отдаёт соответствующую структуру Theme.
 */
class ThemeService {
public:
    void begin();

    // true, если тема реально сменилась
    bool setNight(bool night);

    // текущая тема (цвета)
    const Theme& current() const;

    // геттер состояния (нужен StatusBar)
    bool isNight() const;

private:
    bool _isNight = false;
};