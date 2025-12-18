#include "screens/DummyScreen.h"

DummyScreen::DummyScreen(Adafruit_ST7735& tft)
: _tft(tft) {}

void DummyScreen::begin() {
    _drawn = false;
}

void DummyScreen::update() {
    if (_drawn) return;

    _tft.fillScreen(ST77XX_BLACK);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setTextSize(2);
    _tft.setCursor(10, 40);
    _tft.print("ESP32 Clock v3");

    _drawn = true;
}