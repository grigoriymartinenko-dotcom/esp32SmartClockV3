#include "core/ScreenManager.h"
#include "screens/SettingsScreen.h"
#include "screens/ClockScreen.h"

ScreenManager::ScreenManager(
    Adafruit_ST7735& tft,
    Screen& initial,
    StatusBar& statusBar,
    BottomBar& bottomBar,
    LayoutService& layout,
    UiSeparator& sepStatus,
    UiSeparator& sepBottom,
    UiVersionService& uiVersion
)
: _tft(&tft)
, _current(&initial)
, _statusBar(&statusBar)
, _bottomBar(&bottomBar)
, _layout(&layout)
, _sepStatus(&sepStatus)
, _sepBottom(&sepBottom)
, _uiVersion(&uiVersion)
{}

void ScreenManager::applyLayout() {

    _sepStatus->setY(_layout->sepStatusY());
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

    _statusBar->markDirty();
    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    // 🔥 Settings → Clock → запускаем fade через SCREEN version
    if (_prev && _current) {
        if (_prev->hasStatusBar() == false && _current->hasStatusBar() == true) {
            _uiVersion->bump(UiChannel::SCREEN);
        }
    }

    _layout->setHasBottomBar(_current->hasBottomBar());
    applyLayout();

    _current->begin();

    _statusBar->markDirty();
    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::update() {
    if (_current) {
        _current->update();
    }

    _statusBar->update();

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