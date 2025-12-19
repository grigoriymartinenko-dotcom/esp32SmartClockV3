#include "core/ScreenManager.h"

ScreenManager::ScreenManager(
    Screen& initial,
    StatusBar& statusBar,
    BottomBar& bottomBar
)
: _current(&initial),
  _statusBar(&statusBar),
  _bottomBar(&bottomBar)
{}

void ScreenManager::begin() {
    if (_current) {
        _current->begin();
        _statusBar->markDirty();
        _bottomBar->markDirty();
    }
}

void ScreenManager::update() {
    if (_current) {
        _current->update();
    }
}

void ScreenManager::set(Screen& screen) {
    _current = &screen;
    _current->begin();

    // 💥 КЛЮЧ: бары знают о смене экрана
    _statusBar->markDirty();
    _bottomBar->markDirty();
}

bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}

bool ScreenManager::currentHasBottomBar() const {
    return _current && _current->hasBottomBar();
}