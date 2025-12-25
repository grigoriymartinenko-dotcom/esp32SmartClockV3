#include "screens/settings/WifiListScreen.h"
#include <Adafruit_GFX.h>
#include <cstring>

// ============================================================================
// ctor
// ============================================================================
WifiListScreen::WifiListScreen(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    LayoutService& layout,
    UiVersionService& ui,
    WifiService& wifi,
    PreferencesService& prefs
)
    : Screen(theme)
    , _tft(tft)
    , _layout(layout)
    , _ui(ui)
    , _wifi(wifi)
    , _prefs(prefs)
    , _buttons(tft, theme, layout)
{
}

// ============================================================================
// Screen
// ============================================================================
void WifiListScreen::begin() {

    _state    = State::SCANNING;
    _scroll   = 0;
    _selected = 0;
    _netCount = 0;

    _lastScreenVer = 0;
    _lastWifiVer   = 0;

    // Кнопки: OK / BACK
    _buttons.setVisible(true);
    _buttons.setActions(true, true, true, true);

    _wifi.startScan();
}

void WifiListScreen::update() {

    bool screenDirty = (_lastScreenVer != _ui.version(UiChannel::SCREEN));
    bool wifiDirty   = (_lastWifiVer   != _ui.version(UiChannel::WIFI));

    if (!screenDirty && !wifiDirty)
        return;

    _lastScreenVer = _ui.version(UiChannel::SCREEN);
    _lastWifiVer   = _ui.version(UiChannel::WIFI);

    // ----- determine state -----
    if (_wifi.isScanning()) {
        _state = State::SCANNING;
    }
    else if (_wifi.isScanFinished()) {
        _netCount = _wifi.networksCount();
        _state = (_netCount == 0) ? State::EMPTY : State::READY;
    }
    else {
        _state = State::SCANNING;
    }

    // ----- clear content area -----
    _tft.fillRect(
        0,
        _layout.clockSafeY(),
        _tft.width(),
        _layout.clockSafeH(),
        theme().bg
    );

    drawHeader();

    switch (_state) {
        case State::SCANNING: drawScanning(); break;
        case State::EMPTY:    drawEmpty();    break;
        case State::READY:    drawList();     break;
    }
}

// ============================================================================
// Buttons
// ============================================================================
void WifiListScreen::onShortOk() {

    if (_state != State::READY)
        return;

    int idx = _scroll + _selected;
    if (idx >= _netCount)
        return;

    const char* ssid = _wifi.ssidAt(idx);

    if (isConnectedSsid(ssid)) {
        // OK на подключённой сети → Disconnect
        _wifi.setEnabled(false);
    } else {
        // OK на другой сети → Connect
        _wifi.setEnabled(true);
        _wifi.connect(ssid);
    }

    _ui.bump(UiChannel::WIFI);
}

void WifiListScreen::onShortBack() {
    // handled by controller
}

// ============================================================================
// Drawing
// ============================================================================
void WifiListScreen::drawHeader() {

    _tft.setTextSize(2);
    _tft.setTextColor(theme().textPrimary, theme().bg);
    _tft.setCursor(4, _layout.clockSafeY() - 22);
    _tft.print("Wi-Fi");

    drawSeparator(_layout.clockSafeY() - 2);
}

void WifiListScreen::drawScanning() {

    _tft.setTextSize(1);
    _tft.setTextColor(theme().textSecondary, theme().bg);
    _tft.setCursor(8, _layout.clockSafeY() + 20);
    _tft.print("Scanning...");
}

void WifiListScreen::drawEmpty() {

    _tft.setTextSize(1);
    _tft.setTextColor(theme().textSecondary, theme().bg);
    _tft.setCursor(8, _layout.clockSafeY() + 20);
    _tft.print("No networks found");
}

void WifiListScreen::drawList() {

    const int rowH   = 20;
    const int startY = _layout.clockSafeY() + 4;
    int rows = visibleRows();

    for (int i = 0; i < rows; i++) {

        int idx = _scroll + i;
        if (idx >= _netCount)
            break;

        int y = startY + i * rowH;
        const char* ssid = _wifi.ssidAt(idx);

        bool connected = isConnectedSsid(ssid);

        // SSID
        _tft.setTextSize(1);
        _tft.setTextColor(theme().textPrimary, theme().bg);
        _tft.setCursor(6, y + 6);
        _tft.print(ssid);

        // [Connected]
        if (connected) {
            _tft.setTextColor(theme().accent, theme().bg);
            _tft.setCursor(_tft.width() - 72, y + 6);
            _tft.print("[Connected]");
        }

        if (i < rows - 1 && idx < _netCount - 1) {
            drawSeparator(y + rowH - 1);
        }
    }
}

void WifiListScreen::drawSeparator(int y) {
    _tft.drawFastHLine(0, y, _tft.width(), theme().muted);
}

// ============================================================================
// Helpers
// ============================================================================
int WifiListScreen::visibleRows() const {
    return _layout.clockSafeH() / 20;
}

bool WifiListScreen::isConnectedSsid(const char* ssid) const {
    const char* cur = _wifi.currentSsid();
    return cur && strcmp(cur, ssid) == 0;
}