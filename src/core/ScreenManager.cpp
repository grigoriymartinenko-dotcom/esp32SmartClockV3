#include "core/ScreenManager.h"

// ============================================================================
// ctor
// ============================================================================
ScreenManager::ScreenManager(
    Adafruit_ST7735& tft,
    Screen& initial,
    StatusBar& statusBar,
    //BottomBar& bottomBar,
    ButtonBar& buttonBar,          // 🔥 ДОБАВЛЕНО
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
    //, _bottomBar(&bottomBar)
    , _buttonBar(&buttonBar)       // 🔥 КЛЮЧЕВОЕ МЕСТО
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
    const int h = _layout->statusBarY() + _layout->statusBarH() + 2;

    _tft->fillRect(0, 0, _tft->width(), h, th.bg);
}

void ScreenManager::applyLayout() {

    const bool hasStatus  = (_current && _current->hasStatusBar());
    const bool hasButtons = (_current && _current->hasButtonBar());

    _sepStatus->setVisible(hasStatus);
    _sepStatus->setY(hasStatus
        ? _layout->statusBarY() + _layout->statusBarH()
        : -1
    );
    _sepStatus->markDirty();

    _sepBottom->setVisible(hasButtons);
    _sepBottom->setY(hasButtons
        ? _layout->buttonBarY()
        : -1
    );
    _sepBottom->markDirty();
}

// ============================================================================
// lifecycle
// ============================================================================
void ScreenManager::begin() {
    if (!_current) return;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    applyLayout();

    _current->begin();

    if (wantStatus) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    // BottomBar — legacy, больше не используется
//_bottomBar->setVisible(false);

    _lastTimeVer   = _uiVersion->version(UiChannel::TIME);
    _lastThemeVer  = _uiVersion->version(UiChannel::THEME);
    _lastScreenVer = _uiVersion->version(UiChannel::SCREEN);
}

void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    applyLayout();
    _current->begin();
// 🔑 При смене экрана ButtonBar обязан перерисоваться
if (_buttonBar) {
    _buttonBar->markDirty();
}

    if (wantStatus) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    // BottomBar — legacy, всегда выключен
//_bottomBar->setVisible(false);
    _lastScreenVer = _uiVersion->version(UiChannel::SCREEN);
}

// ============================================================================
// update
// ============================================================================
void ScreenManager::update() {

    if (!_current)
        return;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    //applyLayout();

    if (wantStatus) {
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

        _statusBar->update();
    }

    // ===== ButtonBar (визуальные кнопки) =====
// 1️⃣ СНАЧАЛА экран рисует СВОЙ контент
_current->update();

// 2️⃣ Потом системные разделители
_sepStatus->update();
_sepBottom->update();

// 3️⃣ ПОСЛЕДНИМ — ButtonBar (он фиксирует GFX-состояние)
if (_buttonBar) {
    _buttonBar->setVisible(wantButtons);
    if (wantButtons) {
        _buttonBar->update();
    }
}

}

// ============================================================================
// getters
// ============================================================================
bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}

bool ScreenManager::currentHasBottomBar() const {
    return _current && _current->hasButtonBar();
}