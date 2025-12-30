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

    // 🔑 При старте: системные элементы должны быть в консистентном состоянии
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

    // =========================================================================
    // 🔥 GLOBAL FULL REDRAW (Brightness / аппаратные изменения)
    // -------------------------------------------------------------------------
    // Brightness (PWM подсветки) меняет физическую яркость уже нарисованных пикселей.
    // При частичной отрисовке на TFT остаются "следы" старого кадра.
    //
    // Поэтому:
    //  1) fillScreen(bg) — физически очищаем весь дисплей
    //  2) заставляем текущий экран заново отрисовать ВСЮ свою область (begin)
    //  3) помечаем overlay-элементы dirty (StatusBar/Separators/ButtonBar)
    //
    // Делается ОДИН раз при выходе из BRIGHTNESS (OK/BACK).
    // =========================================================================
    if (_forceFullRedraw) {
        _forceFullRedraw = false;

        if (_tft && _theme) {
            const Theme& th = _theme->current();
            _tft->fillScreen(th.bg);
        }

        // 1) экран пусть заново нарисует свою рабочую область
        if (_current) {
            // begin() у ваших экранов уже умеет делать полный redraw
            _current->begin();
        }

        // 2) overlays пусть перерисуются полностью
        if (_sepStatus) _sepStatus->markDirty();
        if (_sepBottom) _sepBottom->markDirty();
        if (_statusBar) _statusBar->markDirty();
        if (_buttonBar) _buttonBar->markDirty();

        // 3) и дополнительно — bump визуальных каналов (пусть все кеши сбросятся)
        if (_uiVersion) {
            _uiVersion->bump(UiChannel::SCREEN);
            _uiVersion->bump(UiChannel::THEME);
        }
        // ВАЖНО: после этого мы продолжаем обычный update() ниже,
        // чтобы в этом же кадре отрисовались separators/status/buttonbar.
    }

    if (!_current)
        return;

    const bool wantStatus  = _current->hasStatusBar();
    const bool wantButtons = _current->hasButtonBar();

    _layout->setHasStatusBar(wantStatus);
    _layout->setHasBottomBar(wantButtons);

    // =========================================================
    // 1️⃣ СНАЧАЛА экран рисует СВОЙ контент
    // =========================================================
    _current->update();

    // =========================================================
    // 2️⃣ Потом системные разделители (ДОЛЖНЫ обновляться)
    // =========================================================
    _sepStatus->update();
    _sepBottom->update();

    // =========================================================
    // 3️⃣ ПОСЛЕДНИМ — StatusBar (как overlay)
    // =========================================================
    if (wantStatus) {

        // быстрый путь: обновить только строку времени/даты при смене TIME
        if (_uiVersion->changed(UiChannel::TIME)) {
            _statusBar->drawTimeOnly();
        }

        // при смене темы/экрана/вайфая нужно полное обновление статусбара
        if (_uiVersion->changed(UiChannel::THEME) ||
            _uiVersion->changed(UiChannel::SCREEN) ||
            _uiVersion->changed(UiChannel::WIFI))
        {
            _statusBar->markDirty();
        }

        // 🔥 КЛЮЧЕВО: update() обязан вызываться, иначе WiFi/NTP “пропадают”
        _statusBar->update();
    }

    // =========================================================
    // 4️⃣ И СОВСЕМ ПОСЛЕДНИМ — ButtonBar
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
// forceFullRedraw
// ============================================================================
void ScreenManager::forceFullRedraw() {
    _forceFullRedraw = true;
}