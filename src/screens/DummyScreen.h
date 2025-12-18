#pragma once
#include "core/Screen.h"

/*
 * DummyScreen
 * ===========
 * Первый тестовый экран
 */
class DummyScreen : public Screen {
public:
    explicit DummyScreen(Adafruit_ST7735& tft);

    void begin() override;
    void update() override;

private:
    Adafruit_ST7735& _tft;
    bool _drawn = false;
};