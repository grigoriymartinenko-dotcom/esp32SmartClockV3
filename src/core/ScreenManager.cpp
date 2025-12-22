#include "core/ScreenManager.h"

ScreenManager::ScreenManager(
    Adafruit_ST7735& tft,
    Screen& initial,
    StatusBar& statusBar,
    BottomBar& bottomBar,
    LayoutService& layout,
    UiSeparator& sepStatus,
    UiSeparator& sepBottom,
    UiVersionService& uiVersion,
    ThemeService& themeService
)
: _tft(&tft)
, _current(&initial)
, _statusBar(&statusBar)
, _bottomBar(&bottomBar)
, _layout(&layout)
, _sepStatus(&sepStatus)
, _sepBottom(&sepBottom)
, _uiVersion(&uiVersion)
, _theme(&themeService)
{}

// ------------------------------------------------------------
void ScreenManager::clearStatusArea() {
    if (!_tft || !_layout || !_theme) return;

    const Theme& th = _theme->current();

    const int h = _layout->statusY() + _layout->statusH() + 2;

    _tft->fillRect(
        0,
        0,
        _tft->width(),
        h,
        th.bg
    );
}

void ScreenManager::applyLayout() {

    if (_current && _current->hasStatusBar()) {
        _sepStatus->setY(_layout->sepStatusY());
    } else {
        _sepStatus->setY(-1);
    }
    _sepStatus->markDirty();

    if (_current && _current->hasBottomBar()) {
        _sepBottom->setY(_layout->sepBottomY());
    } else {
        _sepBottom->setY(-1);
    }
    _sepBottom->markDirty();
}

void ScreenManager::begin() {
    if (!_current) return;

    _layout->setHasBottomBar(_current->hasBottomBar());
    applyLayout();

    _current->begin();

    if (_current->hasStatusBar()) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    if (_prev && !_prev->hasStatusBar() && _current->hasStatusBar()) {
        _uiVersion->bump(UiChannel::SCREEN);
    }

    _layout->setHasBottomBar(_current->hasBottomBar());
    applyLayout();

    _current->begin();

    if (_current->hasStatusBar()) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::update() {

    if (_current) {
        _current->update();
    }

    if (_current && _current->hasStatusBar()) {
        _statusBar->update();
    }

    _bottomBar->update();

    _sepStatus->update();
    _sepBottom->update();

    if (UiDebugOverlay::isEnabled()) {
        UiDebugOverlay::draw(*_tft);
    }
}

bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}

bool ScreenManager::currentHasBottomBar() const {
    return _current && _current->hasBottomBar();
}