#include "core/ScreenManager.h"

ScreenManager::ScreenManager(Screen& initial)
: _current(&initial)
{
}

void ScreenManager::begin() {
    if (_current) {
        _current->begin();
    }
}

void ScreenManager::update() {
    if (_current) {
        _current->update();
    }
}

void ScreenManager::set(Screen& screen) {
    _current = &screen;
    if (_current) {
        _current->begin();
    }
}