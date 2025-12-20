#include "core/ScreenManager.h"

ScreenManager::ScreenManager(
    Screen& initial,
    StatusBar& statusBar,
    BottomBar& bottomBar,
    LayoutService& layout,
    UiSeparator& sepStatus,
    UiSeparator& sepBottom
)
: _current(&initial)
, _statusBar(&statusBar)
, _bottomBar(&bottomBar)
, _layout(&layout)
, _sepStatus(&sepStatus)
, _sepBottom(&sepBottom)
{}

/*
 * applyLayout()
 * -------------
 * ЕДИНСТВЕННОЕ место, где:
 *  - решается, есть ли нижняя линия
 *  - задаются координаты линий
 */
void ScreenManager::applyLayout() {

    // -------- верхняя линия (ВСЕГДА) --------
    _sepStatus->setY(_layout->sepStatusY());
    _sepStatus->markDirty();

    // -------- нижняя линия --------
    if (_current && _current->hasBottomBar()) {
        _sepBottom->setY(_layout->sepBottomY());
    } else {
        _sepBottom->setY(-1);   // ❗ сигнал "НЕ РИСОВАТЬ"
    }
    _sepBottom->markDirty();
}

void ScreenManager::begin() {
    if (!_current) return;

    // 1. layout знает режим
    _layout->setHasBottomBar(_current->hasBottomBar());

    // 2. линии
    applyLayout();

    // 3. экран
    _current->begin();

    // 4. UI бары
    _statusBar->markDirty();
    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::set(Screen& screen) {
    _current = &screen;

    // 1. layout
    _layout->setHasBottomBar(_current->hasBottomBar());

    // 2. линии
    applyLayout();

    // 3. экран
    _current->begin();

    // 4. UI бары
    _statusBar->markDirty();
    _bottomBar->setVisible(_current->hasBottomBar());
    _bottomBar->markDirty();
}

void ScreenManager::update() {
    if (_current) {
        _current->update();
    }
    _statusBar->update();
}

bool ScreenManager::currentHasStatusBar() const {
    return _current && _current->hasStatusBar();
}

bool ScreenManager::currentHasBottomBar() const {
    return _current && _current->hasBottomBar();
}