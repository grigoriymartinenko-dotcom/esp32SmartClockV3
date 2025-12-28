#include "services/ColorTemperatureService.h"

/*
 * Палитры подбирались:
 *  - без вырвиглаза
 *  - с приоритетом читаемости
 *  - с минимальным цветовым шумом
 *
 * Это НЕ "тема", а именно температурный сдвиг.
 *
 * ВАЖНОЕ отличие от "палитр":
 *  - раньше мы возвращали готовые фиксированные цвета (text/icon/secondary)
 *  - теперь основной API — apply(ThemeBlend): фильтр, который можно наложить
 *    на ЛЮБУЮ тему и любые семантические цвета.
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
// MAIN API: post-process over ThemeBlend
// ============================================================================
ThemeBlend ColorTemperatureService::apply(const ThemeBlend& in) const {

    // DAY — быстрый путь: ничего не меняем
    if (_current == ColorTemp::DAY) {
        return in;
    }

    uint16_t rMul256, gMul256, bMul256;
    multipliers(rMul256, gMul256, bMul256);

    ThemeBlend out = in;

    /*
     * Фон (bg) мы намеренно НЕ трогаем:
     *  - фон часто занимает большую площадь
     *  - любые микросдвиги фона сильнее заметны как "желе" и мерцание
     *  - лучше греть именно контент (текст/иконки/акценты)
     */

    out.fg      = scale565(in.fg,      rMul256, gMul256, bMul256);
    out.accent  = scale565(in.accent,  rMul256, gMul256, bMul256);
    out.muted   = scale565(in.muted,   rMul256, gMul256, bMul256);
    out.warn    = scale565(in.warn,    rMul256, gMul256, bMul256);
    out.success = scale565(in.success, rMul256, gMul256, bMul256);

    return out;
}

// ============================================================================
// legacy public colors (оставлены как совместимость)
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
    return (uint16_t)(((r & 0xF8) << 8)
         | ((g & 0xFC) << 3)
         |  (b >> 3));
}

uint16_t ColorTemperatureService::scale565(uint16_t c, uint16_t rMul256, uint16_t gMul256, uint16_t bMul256) {
    // Извлекаем RGB565
    uint16_t r = (c >> 11) & 0x1F; // 0..31
    uint16_t g = (c >> 5)  & 0x3F; // 0..63
    uint16_t b =  c        & 0x1F; // 0..31

    // Масштабируем в fixed-point (mul256)
    // r/g/b остаются в "5/6/5 бит", поэтому умножаем и делим обратно
    // Важно: деление через >>8 (так как mul256)
    r = (uint16_t)((r * rMul256) >> 8);
    g = (uint16_t)((g * gMul256) >> 8);
    b = (uint16_t)((b * bMul256) >> 8);

    // Клэмп (на всякий, хотя при mul<=256 обычно не переполняется)
    if (r > 31) r = 31;
    if (g > 63) g = 63;
    if (b > 31) b = 31;

    // Собираем обратно RGB565
    return (uint16_t)((r << 11) | (g << 5) | b);
}

void ColorTemperatureService::multipliers(uint16_t& rMul256, uint16_t& gMul256, uint16_t& bMul256) const {
    /*
     * Коэффициенты берём из твоих "температурных белых":
     *  - DAY:     255,255,255  -> 1.0, 1.0, 1.0
     *  - EVENING: 255,235,210  -> 1.0, 0.92, 0.82
     *  - NIGHT:   255,210,170  -> 1.0, 0.82, 0.67
     *
     * В fixed-point (mul256):
     *  - 1.0    = 256
     *  - 235/255*256 ≈ 236
     *  - 210/255*256 ≈ 211
     *  - 170/255*256 ≈ 171
     */

    switch (_current) {
        case ColorTemp::EVENING:
            rMul256 = 256;
            gMul256 = (uint16_t)((235 * 256) / 255);
            bMul256 = (uint16_t)((210 * 256) / 255);
            return;

        case ColorTemp::NIGHT:
            rMul256 = 256;
            gMul256 = (uint16_t)((210 * 256) / 255);
            bMul256 = (uint16_t)((170 * 256) / 255);
            return;

        case ColorTemp::DAY:
        default:
            rMul256 = 256;
            gMul256 = 256;
            bMul256 = 256;
            return;
    }
}