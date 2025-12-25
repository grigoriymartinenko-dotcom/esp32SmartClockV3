#include "screens/settings/WifiListScreen.h"

#include <Adafruit_GFX.h>

// ============================================================================
// ctor
// ============================================================================
WifiListScreen::WifiListScreen(
    Adafruit_ST7735& tft,
    ThemeService& theme,
    LayoutService& layout,
    UiVersionService& ui,
    WifiService& wifi
)
    : Screen(theme)
    , _tft(tft)
    , _layout(layout)
    , _ui(ui)
    , _wifi(wifi)
    , _buttons(tft, theme, layout)
{
}

// ============================================================================
// Screen
// ============================================================================
void WifiListScreen::begin() {
    _state    = State::SCANNING;
    _scroll   = 0;
    _netCount = 0;

    _lastScreenVer = 0;
    _lastWifiVer   = 0;

    _buttons.setVisible(true);

    _wifi.startScan();
}

void WifiListScreen::update() {
    bool screenDirty = (_lastScreenVer != _ui.version(UiChannel::SCREEN));
    bool wifiDirty   = (_lastWifiVer   != _ui.version(UiChannel::WIFI));

    if (!screenDirty && !wifiDirty) {
        return;
    }

    _lastScreenVer = _ui.version(UiChannel::SCREEN);
    _lastWifiVer   = _ui.version(UiChannel::WIFI);

    // determine state
    if (_wifi.isScanning()) {
        _state = State::SCANNING;
    } else if (_wifi.isScanFinished()) {
        _netCount = _wifi.networksCount();
        _state = (_netCount == 0) ? State::EMPTY : State::READY;
    } else {
        _state = State::SCANNING;
    }

    // redraw
    _tft.fillScreen(theme().bg);
    drawHeader();

    switch (_state) {
        case State::SCANNING: drawScanning(); break;
        case State::EMPTY:    drawEmpty();    break;
        case State::READY:    drawList();     break;
    }
}

// ============================================================================
// Drawing
// ============================================================================
void WifiListScreen::drawHeader() {
    _tft.setTextSize(2);
    _tft.setTextColor(theme().textPrimary, theme().bg);
    _tft.setCursor(4, 4);
    _tft.print("Wi-Fi");

    drawSeparator(24);
}

void WifiListScreen::drawScanning() {
    _tft.setTextSize(1);
    _tft.setTextColor(theme().textSecondary, theme().bg);
    _tft.setCursor(8, 40);
    _tft.print("Scanning...");
}

void WifiListScreen::drawEmpty() {
    _tft.setTextSize(1);
    _tft.setTextColor(theme().textSecondary, theme().bg);
    _tft.setCursor(8, 40);
    _tft.print("No networks found");
}

void WifiListScreen::drawList() {
    const int rowH   = 20;
    const int startY = 26;
    int rows = visibleRows();

    for (int i = 0; i < rows; i++) {
        int idx = _scroll + i;
        if (idx >= _netCount) break;

        int y = startY + i * rowH;

        const char* ssid = _wifi.ssidAt(idx);

        _tft.setTextSize(1);
        _tft.setTextColor(theme().textPrimary, theme().bg);
        _tft.setCursor(6, y + 6);
        _tft.print(ssid);

        // separator between items (no last)
        if (i < rows - 1 && idx < _netCount - 1) {
            drawSeparator(y + rowH - 1);
        }
    }
}

void WifiListScreen::drawSeparator(int y) {
    _tft.drawFastHLine(0, y, _tft.width(), theme().textSecondary);
}

// ============================================================================
// Helpers
// ============================================================================
int WifiListScreen::visibleRows() const {
    return (_layout.clockH() - 26) / 20;
}