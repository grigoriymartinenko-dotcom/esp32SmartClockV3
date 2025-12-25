#include "core/ScreenManager.h"

// ============================================================================
// ctor
// ============================================================================
ScreenManager::ScreenManager(
    Adafruit_ST7735& tft,
    Screen& initial,
    StatusBar& statusBar,
    BottomBar& bottomBar,
    ButtonBar& buttonBar,
    LayoutService& layout,
    UiSeparator& sepStatus,
    UiSeparator& sepBottom,
    UiVersionService& uiVersion,
    ThemeService& themeService
)
    : _tft(&tft)
    , _current(&initial)
    , _prev(nullptr)
    , _statusBar(&statusBar)
    , _bottomBar(&bottomBar)   // legacy, но выключен
    , _buttonBar(&buttonBar)
    , _layout(&layout)
    , _sepStatus(&sepStatus)
    , _sepBottom(&sepBottom)
    , _uiVersion(&uiVersion)
    , _theme(&themeService)
{
}

// ============================================================================
// helpers
// ============================================================================
void ScreenManager::clearStatusArea() {
    if (!_tft || !_layout || !_theme) return;

    const Theme& th = _theme->current();
    int h = _layout->statusY() + _layout->statusH() + 2;

    _tft->fillRect(0, 0, _tft->width(), h, th.bg);
}

void ScreenManager::applyLayout() {

    // ===== Status separator =====
    if (_current && _current->hasStatusBar()) {
        _sepStatus->setY(_layout->statusY() + _layout->statusH());
    } else {
        _sepStatus->setY(-1);
    }
    _sepStatus->markDirty();

    // ===== ButtonBar separator =====
    if (_current && _current->hasButtonBar()) {
        _sepBottom->setY(_layout->buttonBarY());
    } else {
        _sepBottom->setY(-1);
    }
    _sepBottom->markDirty();
}

// ============================================================================
// lifecycle
// ============================================================================
void ScreenManager::begin() {
    if (!_current) return;

    // Layout — только StatusBar + ButtonBar
    _layout->setHasStatusBar(_current->hasStatusBar());
    _layout->setHasButtonBar(_current->hasButtonBar());

    applyLayout();
    _current->begin();

    // StatusBar
    if (_current->hasStatusBar()) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    // ❌ BottomBar ВЫКЛЮЧЕН ВСЕГДА
    _bottomBar->setVisible(false);

    // ButtonBar — единый и стабильный
    _buttonBar->setVisible(_current->hasButtonBar());
    _buttonBar->markDirty();

    _lastTimeVer   = _uiVersion->version(UiChannel::TIME);
    _lastThemeVer  = _uiVersion->version(UiChannel::THEME);
    _lastScreenVer = _uiVersion->version(UiChannel::SCREEN);
}

void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    _layout->setHasStatusBar(_current->hasStatusBar());
    _layout->setHasButtonBar(_current->hasButtonBar());

    applyLayout();
    _current->begin();

    if (_current->hasStatusBar()) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    _bottomBar->setVisible(false);

    _buttonBar->setVisible(_current->hasButtonBar());
    _buttonBar->markDirty();

    _lastScreenVer = _uiVersion->version(UiChannel::SCREEN);
}

// ============================================================================
// update
// ============================================================================
void ScreenManager::update() {

    if (_current) {
        _current->update();
    }

    // StatusBar versioning
    if (_current && _current->hasStatusBar()) {

        uint32_t v;

        v = _uiVersion->version(UiChannel::TIME);
        if (v != _lastTimeVer) {
            _lastTimeVer = v;
            _statusBar->markDirty();
        }

        v = _uiVersion->version(UiChannel::THEME);
        if (v != _lastThemeVer) {
            _lastThemeVer = v;
            _statusBar->markDirty();
        }

        v = _uiVersion->version(UiChannel::SCREEN);
        if (v != _lastScreenVer) {
            _lastScreenVer = v;
            _statusBar->markDirty();
        }
    }

    if (_current && _current->hasStatusBar()) {
        _statusBar->update();
    }

    // ❌ BottomBar НЕ используется
    // _bottomBar->update();

    // ButtonBar — ВСЕГДА один и тот же
    const bool wantButtons = _current && _current->hasButtonBar();
    _layout->setHasButtonBar(wantButtons);
    _buttonBar->setVisible(wantButtons);
    _buttonBar->update();

    _sepStatus->update();
    _sepBottom->update();

    if (UiDebugOverlay::isEnabled()) {
        UiDebugOverlay::draw(*_tft);
    }
}

// ============================================================================
// getters
// ============================================================================
bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}

bool ScreenManager::currentHasBottomBar() const {
    return false; // legacy отключён
}