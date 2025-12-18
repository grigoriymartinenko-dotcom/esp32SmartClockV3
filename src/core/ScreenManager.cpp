#include "core/ScreenManager.h"

void ScreenManager::set(Screen* screen) {
    _active = screen;
    if (_active) {
        _active->begin();
    }
}

Screen* ScreenManager::active() const {
    return _active;
}

void ScreenManager::begin() {
    if (_active) {
        _active->begin();
    }
}

void ScreenManager::update() {
    if (_active) {
        _active->update();
    }
}