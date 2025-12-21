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

    // Верхний разделитель (под StatusBar)
    _sepStatus->setY(_layout->sepStatusY());
    _sepStatus->markDirty();

    // Нижний разделитель существует ТОЛЬКО если BottomBar включён на текущем экране
    if (_current && _current->hasBottomBar()) {
        _sepBottom->setY(_layout->sepBottomY());
    } else {
        _sepBottom->setY(-1); // скрываем
    }
    _sepBottom->markDirty();
}

void ScreenManager::begin() {
    if (!_current) return;

    // Layout зависит от того, нужен ли BottomBar
    _layout->setHasBottomBar(_current->hasBottomBar());
    applyLayout();

    // Экран сам рисует свою область
    _current->begin();

    // Общие UI элементы
    _statusBar->markDirty();

    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::set(Screen& screen) {

    _prev = _current;
    _current = &screen;

    // 🔥 Settings → Clock → запускаем fade через SCREEN version
    // (пример: возврат со Settings, где статусбара нет, на экран со статусбаром)
    if (_prev && _current) {
        if (_prev->hasStatusBar() == false && _current->hasStatusBar() == true) {
            _uiVersion->bump(UiChannel::SCREEN);
        }
    }

    // Layout/разделители должны соответствовать новому экрану
    _layout->setHasBottomBar(_current->hasBottomBar());
    applyLayout();

    // Новый экран рисует себя с нуля в begin()
    _current->begin();

    // Общие UI элементы
    _statusBar->markDirty();

    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::update() {
    // =========================================================
    // ЕДИНСТВЕННАЯ точка отрисовки всего UI.
    // Это важно: иначе появляются двойные update(), мерцания и хаос.
    // =========================================================

    // 1) Активный экран
    if (_current) {
        _current->update();
    }

    // 2) StatusBar (верх)
    _statusBar->update();

    // 3) BottomBar (низ)
    // ВАЖНО:
    // Даже если BottomBar скрыт, update() нужен, чтобы он смог
    // ОДИН раз стереть свою область при переходе (см. BottomBar::clear()).
    _bottomBar->update();

    // 4) Разделители (верх/низ)
    // Они живут отдельно от экранов и должны обновляться централизованно.
    _sepStatus->update();
    _sepBottom->update();

    // 5) Debug overlay
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