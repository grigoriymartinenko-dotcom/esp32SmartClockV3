#pragma once
#include <Arduino.h>
#include <Adafruit_ST7735.h>

/*
 * Базовый экран
 * ==============
 * Все экраны наследуются отсюда
 */
class Screen {
public:
    virtual ~Screen() = default;

    virtual void begin() {}
    virtual void update() {}

    // кнопки (пока заглушки)
    virtual void onUp() {}
    virtual void onDown() {}
    virtual void onOk() {}
    virtual void onBack() {}
};