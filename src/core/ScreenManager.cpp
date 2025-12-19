#include "core/ScreenManager.h"

ScreenManager::ScreenManager(Screen& initial)
: _current(&initial)
{}

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
    if (_current == &screen)
        return;

    _current = &screen;
    if (_current) {
        _current->begin();
    }
}

void ScreenManager::notifyThemeChanged() {
    if (_current) {
        _current->onThemeChanged();
    }
}

bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}