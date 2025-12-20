#pragma once
#include <Arduino.h>

static inline uint16_t blend565(uint16_t bg, uint16_t fg, uint8_t a /*0..255*/) {
    // a=0 -> bg, a=255 -> fg
    uint8_t bg_r = (bg >> 11) & 0x1F;
    uint8_t bg_g = (bg >> 5)  & 0x3F;
    uint8_t bg_b =  bg        & 0x1F;

    uint8_t fg_r = (fg >> 11) & 0x1F;
    uint8_t fg_g = (fg >> 5)  & 0x3F;
    uint8_t fg_b =  fg        & 0x1F;

    uint8_t r = (uint16_t(bg_r) * (255 - a) + uint16_t(fg_r) * a) / 255;
    uint8_t g = (uint16_t(bg_g) * (255 - a) + uint16_t(fg_g) * a) / 255;
    uint8_t b = (uint16_t(bg_b) * (255 - a) + uint16_t(fg_b) * a) / 255;

    return (uint16_t(r) << 11) | (uint16_t(g) << 5) | uint16_t(b);
}