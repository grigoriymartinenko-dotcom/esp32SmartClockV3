#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "core/ScreenManager.h"
#include "ui/StatusBar.h"
#include "ui/Theme.h"
#include "core/TimeService.h"
#include "core/NightService.h"
#include "screens/ClockScreen.h"

// TFT pins (оставь свои)
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

ScreenManager screenManager;

TimeService  timeService;
NightService nightService;

StatusBar statusBar(tft);
ClockScreen clockScreen(tft, timeService, nightService);

void setup() {
    Serial.begin(115200);
    delay(200);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(C_UI_BG_DAY);

timeService.begin();
    nightService.setMode(NightService::Mode::AUTO);

    screenManager.set(&clockScreen);
}

void loop() {
    // обновление экранов
    screenManager.update();
timeService.update();
nightService.update(timeService.hour());

// прокидываем night в статусбар
statusBar.setNight(nightService.isNight());
    // Рисуем статусбар (топ-полоса)
    // x,y = 0,0
    // w = ширина экрана
    // h = 12 пикс
    statusBar.draw(0, 0, tft.width(), 12);

    delay(20);
}