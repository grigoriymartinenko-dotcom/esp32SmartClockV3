#pragma once
#include <Arduino.h>

/*
 * WeatherIcons.h
 * ---------------
 * UI-helper для погодных иконок.
 * Возвращает bitmap + размер (теперь 16×16).
 */

struct WeatherIcon {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
};

WeatherIcon getWeatherIcon(int weatherCode, bool night);