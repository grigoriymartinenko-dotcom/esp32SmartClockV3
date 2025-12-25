#include "screens/SettingsScreen.h"
#include <cstring>

extern PreferencesService prefs;

/*
 * SettingsWifi.cpp
 * ----------------
 * ЛОГИКА Wi-Fi настроек.
 *
 * Этот файл:
 *  - НЕ рисует UI
 *  - НЕ знает координат экрана
 *  - НЕ использует BottomBar
 *
 * Он работает ТОЛЬКО с:
 *  - состояниями SettingsScreen
 *  - сервисом WifiService
 *  - PreferencesService
 *
 * Вся отрисовка выполняется в SettingsScreen::draw*()
 *
 * ---------------------------------------------------------------------------
 * СЦЕНАРИИ:
 *
 * 1) ROOT → WIFI
 *    - пункт "Wi-Fi"
 *
 * 2) WIFI
 *    - [0] Wi-Fi ON/OFF (пока не редактируем здесь)
 *    - [1] Scan → запуск сканирования → WIFI_LIST
 *
 * 3) WIFI_LIST
 *    - список сетей + пункт "Rescan"
 *    - выбор сети → WIFI_PASSWORD
 *
 * 4) WIFI_PASSWORD
 *    - ввод пароля по символам
 *    - OK  → добавить символ
 *    - BACK(short) → backspace
 *    - OK(long) → сохранить + подключиться
 *    - BACK(long) → отмена → WIFI_LIST
 */

// ============================================================================
// WIFI: Short OK
// ----------------------------------------------------------------------------
// Обрабатывает КОРОТКОЕ нажатие OK в Wi-Fi экранах
// ============================================================================
bool SettingsScreen::handleWifiShortOk() {

    // ------------------------------------------------------------------------
    // LEVEL: WIFI
    // ------------------------------------------------------------------------
    // Подменю Wi-Fi:
    //  subSelected = 0 → Wi-Fi ON/OFF (пока не редактируем)
    //  subSelected = 1 → Scan
    if (_level == Level::WIFI) {

        // Запуск сканирования
        if (_subSelected == 1) {
            _wifi.startScan();

            // Сброс навигации списка
            _wifiListSelected = 0;
            _wifiListTop      = 0;

            // Переход к списку сетей
            enterSubmenu(Level::WIFI_LIST);
            return true;
        }

        // Любое другое действие — просто перерисовать
        _dirty = true;
        return true;
    }

    // ------------------------------------------------------------------------
    // LEVEL: WIFI_LIST
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_LIST) {

        // Пока идёт сканирование — ничего не выбираем
        if (_wifi.isScanning()) {
            _dirty = true;
            return true;
        }

        const int netCount = _wifi.networksCount();

        // Нет сетей → повторный Scan
        if (netCount <= 0) {
            _wifi.startScan();
            _dirty = true;
            return true;
        }

        // Последний пункт списка — Rescan
        if (_wifiListSelected == netCount) {
            _wifi.startScan();
            _dirty = true;
            return true;
        }

        // Выбрана сеть → ввод пароля
        memset(_wifiPass, 0, sizeof(_wifiPass));
        _wifiPassLen = 0;
        _wifiCharIdx = 0;

        enterSubmenu(Level::WIFI_PASSWORD);
        return true;
    }

    // ------------------------------------------------------------------------
    // LEVEL: WIFI_PASSWORD
    // ------------------------------------------------------------------------
    // OK → добавить текущий символ в пароль
    if (_level == Level::WIFI_PASSWORD) {

        if (_wifiPassLen < WIFI_PASS_MAX) {
            _wifiPass[_wifiPassLen++] = PASS_CHARS[_wifiCharIdx];
            _wifiPass[_wifiPassLen]   = 0;
            _dirty = true;
        }
        return true;
    }

    return false;
}

// ============================================================================
// WIFI: Short BACK
// ----------------------------------------------------------------------------
// КОРОТКОЕ BACK
// ============================================================================
bool SettingsScreen::handleWifiShortBack() {

    // ------------------------------------------------------------------------
    // LEVEL: WIFI_PASSWORD
    // ------------------------------------------------------------------------
    if (_level == Level::WIFI_PASSWORD) {

        // Есть символы → backspace
        if (_wifiPassLen > 0) {
            _wifiPass[--_wifiPassLen] = 0;
            _dirty = true;
            return true;
        }

        // Пароль пуст → возврат к списку сетей
        enterSubmenu(Level::WIFI_LIST);
        return true;
    }

    return false;
}

// ============================================================================
// WIFI: Long OK
// ----------------------------------------------------------------------------
// ДЛИННОЕ OK → APPLY
// ============================================================================
bool SettingsScreen::handleWifiLongOk() {

    // Работает ТОЛЬКО в режиме ввода пароля
    if (_level != Level::WIFI_PASSWORD)
        return false;

    const char* ssid = _wifi.ssidAt(_wifiListSelected);

    // Некорректный SSID
    if (!ssid || !ssid[0]) {
        _dirty = true;
        return true;
    }

    // ------------------------------------------------------------------------
    // 1) Сохранение учётных данных
    // ------------------------------------------------------------------------
    prefs.setWifiCredentials(ssid, _wifiPass);
    prefs.save();

    // ------------------------------------------------------------------------
    // 2) Подключение
    // ------------------------------------------------------------------------
    _wifi.connect(ssid, _wifiPass);

    // ------------------------------------------------------------------------
    // 3) Возврат к списку (там видим статус подключения)
    // ------------------------------------------------------------------------
    enterSubmenu(Level::WIFI_LIST);
    return true;
}

// ============================================================================
// WIFI: Long BACK
// ----------------------------------------------------------------------------
// ДЛИННОЕ BACK → CANCEL
// ============================================================================
bool SettingsScreen::handleWifiLongBack() {

    if (_level == Level::WIFI_PASSWORD) {
        // Отмена ввода пароля
        enterSubmenu(Level::WIFI_LIST);
        return true;
    }

    return false;
}