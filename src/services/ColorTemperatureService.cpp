#include "services/ColorTemperatureService.h"

/*
 * Палитры подбирались:
 *  - без вырвиглаза
 *  - с приоритетом читаемости
 *  - с минимальным цветовым шумом
 *
 * Это НЕ "тема", а именно температурный сдвиг.
 */

// ============================================================================
// ctor
// ============================================================================
ColorTemperatureService::ColorTemperatureService()
    : _current(ColorTemp::DAY)
{
}

// ============================================================================
// control
// ============================================================================
void ColorTemperatureService::set(ColorTemp t) {
    _current = t;
}

ColorTemp ColorTemperatureService::current() const {
    return _current;
}

// ============================================================================
// public colors
// ============================================================================
uint16_t ColorTemperatureService::text() const {
    switch (_current) {
        case ColorTemp::DAY:
            return rgb565(255, 255, 255);   // чистый белый

        case ColorTemp::EVENING:
            return rgb565(255, 235, 210);   // мягкий тёплый

        case ColorTemp::NIGHT:
            return rgb565(255, 210, 170);   // очень тёплый, но читаемый
    }
    return rgb565(255, 255, 255);
}

uint16_t ColorTemperatureService::icon() const {
    switch (_current) {
        case ColorTemp::DAY:
            return rgb565(230, 230, 230);

        case ColorTemp::EVENING:
            return rgb565(230, 215, 195);

        case ColorTemp::NIGHT:
            return rgb565(210, 180, 150);
    }
    return rgb565(230, 230, 230);
}

uint16_t ColorTemperatureService::secondary() const {
    switch (_current) {
        case ColorTemp::DAY:
            return rgb565(180, 180, 180);

        case ColorTemp::EVENING:
            return rgb565(180, 165, 145);

        case ColorTemp::NIGHT:
            return rgb565(160, 135, 115);
    }
    return rgb565(180, 180, 180);
}

// ============================================================================
// helpers
// ============================================================================
uint16_t ColorTemperatureService::rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8)
         | ((g & 0xFC) << 3)
         |  (b >> 3);
}