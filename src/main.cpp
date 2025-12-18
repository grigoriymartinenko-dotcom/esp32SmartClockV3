#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "core/ScreenManager.h"
#include "screens/DummyScreen.h"

/* ===== TFT PINS (подставь свои при необходимости) ===== */
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

ScreenManager screenManager;
DummyScreen dummyScreen(tft);

void setup() {
    Serial.begin(115200);
    delay(200);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);

    screenManager.set(&dummyScreen);
}

void loop() {
    screenManager.update();
    delay(20);
}