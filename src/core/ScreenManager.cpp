#include "core/ScreenManager.h"

// ============================================================================
// ctor
// ============================================================================
ScreenManager::ScreenManager(
    Adafruit_ST7735& tft,
    Screen& initial,
    StatusBar& statusBar,
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
    const int h = _layout->statusBarY() + _layout->statusBarH();

    _tft->fillRect(0, 0, _tft->width(), h, th.bg);
}

void ScreenManager::applyLayout() {

    const bool hasStatus  = (_current && _current->hasStatusBar());
    const bool hasButtons = (_current && _current->hasButtonBar());

    _sepStatus->setVisible(hasStatus);
    _sepStatus->setY(
        hasStatus
            ? _layout->statusBarY() + _layout->statusBarH()
            : -1
    );
    _sepStatus->markDirty();

    _sepBottom->setVisible(hasButtons);
    _sepBottom->setY(
        hasButtons
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

    // Экран полностью рисует свою область
    _current->begin();

    // Overlay-элементы
    if (wantStatus) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }

    if (_buttonBar) {
        _buttonBar->markDirty();
    }
}

// ============================================================================
// screen switch
// ============================================================================
void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    applyLayout();

    _current->begin();

    // Обязательный redraw overlays
    if (_buttonBar) {
        _buttonBar->markDirty();
    }

    if (wantStatus) {
        _statusBar->markDirty();
    } else {
        clearStatusArea();
    }
}

// ============================================================================
// update (ГЛАВНЫЙ UI LOOP)
// ============================================================================
void ScreenManager::update() {

    if (!_current)
        return;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    // =========================================================
    // 1️⃣ СНАЧАЛА — основной экран
    // =========================================================
    _current->update();

    // =========================================================
    // 2️⃣ Разделители (если видимы)
    // =========================================================
    if (_sepStatus) _sepStatus->update();
    if (_sepBottom) _sepBottom->update();

    // =========================================================
    // 3️⃣ ПОСЛЕДНИМ — StatusBar (overlay)
    //
    // 🔥 КЛЮЧЕВО:
    //  - update() вызывается ВСЕГДА
    //  - НИКАКИХ UiVersion::changed() здесь
    //  - StatusBar сам решает, dirty он или нет
    // =========================================================
    if (wantStatus && _statusBar) {
        _statusBar->update();
    }

    // =========================================================
    // 4️⃣ ButtonBar — самый верхний слой
    // =========================================================
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

// ============================================================================
// force full redraw (используется, например, после Brightness)
// ============================================================================
void ScreenManager::forceFullRedraw() {
    if (!_tft || !_theme) return;

    const Theme& th = _theme->current();
    _tft->fillScreen(th.bg);

    if (_current) {
        _current->begin();
    }

    if (_sepStatus)  _sepStatus->markDirty();
    if (_sepBottom)  _sepBottom->markDirty();
    if (_statusBar)  _statusBar->markDirty();
    if (_buttonBar)  _buttonBar->markDirty();
}