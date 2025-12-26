#include "screens/settings/WifiListScreen.h"
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <cstdio>

// ============================================================================
// UI constants
// ============================================================================
static constexpr int HEADER_H        = 24;
static constexpr int STATUS_H        = 16;
static constexpr int ROW_H           = 20;
static constexpr int LIST_PAD_X      = 6;
static constexpr int LIST_PAD_Y      = 4;
static constexpr int ROW_TEXT_Y_OFF  = 6;

// ============================================================================
// small text helpers
// ============================================================================
static void getTextWH(Adafruit_ST7735& tft, const char* s, int16_t& w, int16_t& h) {
    int16_t x1, y1;
    uint16_t ww, hh;
    tft.getTextBounds(s, 0, 0, &x1, &y1, &ww, &hh);
    w = (int16_t)ww;
    h = (int16_t)hh;
}

static void drawCenteredText1(
    Adafruit_ST7735& tft,
    int16_t y,
    const char* text,
    uint16_t fg,
    uint16_t bg
) {
    tft.setTextSize(1);
    tft.setTextColor(fg, bg);

    int16_t tw = 0, th = 0;
    getTextWH(tft, text, tw, th);

    int16_t x = (int16_t)((tft.width() - tw) / 2);
    if (x < 0) x = 0;

    tft.setCursor(x, y);
    tft.print(text);
}

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

    _tft.fillScreen(theme().bg);

    _tft.fillRect(
        0,
        _layout.buttonBarY(),
        _tft.width(),
        _layout.buttonBarH(),
        theme().bg
    );

    _state    = State::SCANNING;
    _scroll   = 0;
    _selected = 0;
    _netCount = 0;

    _lastScreenVer = 0;
    _lastWifiVer   = 0;

    _lastConnState = _wifi.state();
    const char* ssid = _wifi.currentSsid();
    if (ssid) {
        strncpy(_lastConnSsid, ssid, sizeof(_lastConnSsid) - 1);
        _lastConnSsid[sizeof(_lastConnSsid) - 1] = '\0';
    } else {
        _lastConnSsid[0] = '\0';
    }

    _buttons.setVisible(true);
    _buttons.setActions(true, true, true, true);
    _buttons.setHighlight(true, true, true, true);
    _layout.setHasBottomBar(true);

    _wifi.startScan();

    _ui.bump(UiChannel::SCREEN);
    _ui.bump(UiChannel::WIFI);
}

bool WifiListScreen::connectionModelDirty() {
    const WifiService::State st = _wifi.state();
    const char* ssid = _wifi.currentSsid();
    const char* s = ssid ? ssid : "";

    if (st != _lastConnState) {
        _lastConnState = st;
        strncpy(_lastConnSsid, s, sizeof(_lastConnSsid) - 1);
        _lastConnSsid[sizeof(_lastConnSsid) - 1] = '\0';
        return true;
    }

    if (strncmp(_lastConnSsid, s, sizeof(_lastConnSsid)) != 0) {
        strncpy(_lastConnSsid, s, sizeof(_lastConnSsid) - 1);
        _lastConnSsid[sizeof(_lastConnSsid) - 1] = '\0';
        return true;
    }

    return false;
}

void WifiListScreen::update() {

    const uint32_t scrV = _ui.version(UiChannel::SCREEN);
    const uint32_t wfiV = _ui.version(UiChannel::WIFI);

    const bool screenDirty = (_lastScreenVer != scrV);
    const bool wifiDirty   = (_lastWifiVer   != wfiV);

    State newState = _state;
    int   newCount = _netCount;

    if (_wifi.isScanning()) {
        newState = State::SCANNING;
    }
    else if (_wifi.isScanFinished()) {
        newCount = _wifi.networksCount();
        newState = (newCount == 0) ? State::EMPTY : State::READY;
    }
    else {
        newState = State::SCANNING;
    }

    const bool listModelDirty = (newState != _state) || (newCount != _netCount);
    const bool connDirty = connectionModelDirty();

    if (!screenDirty && !wifiDirty && !listModelDirty && !connDirty)
        return;

    _lastScreenVer = scrV;
    _lastWifiVer   = wfiV;

    _state    = newState;
    _netCount = newCount;

    const int baseY = _layout.contentY();

    _tft.fillRect(
        0,
        baseY,
        _tft.width(),
        _layout.contentH(),
        theme().bg
    );

    drawHeader(baseY);
    drawConnectionStatus(baseY);

    switch (_state) {
        case State::SCANNING: drawScanning(baseY); break;
        case State::EMPTY:    drawEmpty(baseY);    break;
        case State::READY:    drawList(baseY);     break;
    }
    _buttons.update();
}

// ============================================================================
// Buttons
// ============================================================================
void WifiListScreen::onShortLeft() {

    if (_state != State::READY)
        return;

    if (_selected > 0) {
        _selected--;
    } else if (_scroll > 0) {
        _scroll--;
    }

    _ui.bump(UiChannel::SCREEN);
}

void WifiListScreen::onShortRight() {

    if (_state != State::READY)
        return;

    int rows = visibleRows();

    if (_selected < rows - 1 && (_scroll + _selected + 1) < _netCount) {
        _selected++;
    } else if ((_scroll + rows) < _netCount) {
        _scroll++;
    }

    _ui.bump(UiChannel::SCREEN);
}

void WifiListScreen::onShortOk() {

    if (_state != State::READY)
        return;

    int idx = _scroll + _selected;
    if (idx >= _netCount)
        return;

    const char* ssid = _wifi.ssidAt(idx);

    if (isConnectedSsid(ssid)) {
        _wifi.setEnabled(false);
    } else {
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
void WifiListScreen::drawHeader(int baseY) {

    _tft.setTextSize(2);
    _tft.setTextColor(theme().textPrimary, theme().bg);
    _tft.setCursor(4, baseY + 4);
    _tft.print("Wi-Fi Scan");

    drawSeparator(baseY + HEADER_H - 1);
}

void WifiListScreen::drawConnectionStatus(int baseY) {

    const int y = baseY + HEADER_H;

    _tft.fillRect(0, y, _tft.width(), STATUS_H, theme().bg);

    const WifiService::State st = _wifi.state();

    const char* ssid = _wifi.currentSsid();
    const char* text = nullptr;
    uint16_t color   = theme().textSecondary;

    if (st == WifiService::State::ONLINE && ssid) {
        static char buf[40];
        snprintf(buf, sizeof(buf), "Connected: %s", ssid);
        text  = buf;
        color = theme().accent;
    }
    else if (st == WifiService::State::CONNECTING) {
        text  = "Reconnecting...";
        color = theme().warn;
    }
    else {
        text  = "Not connected";
        color = theme().textSecondary;
    }

    drawCenteredText1(_tft, y + 3, text, color, theme().bg);
    drawSeparator(y + STATUS_H - 1);
}

void WifiListScreen::drawScanning(int baseY) {
    int y = baseY + HEADER_H + STATUS_H + 10;
    drawCenteredText1(_tft, y, "Scanning...", theme().textSecondary, theme().bg);
}

void WifiListScreen::drawEmpty(int baseY) {
    int y = baseY + HEADER_H + STATUS_H + 10;
    drawCenteredText1(_tft, y, "No networks found", theme().textSecondary, theme().bg);
}

void WifiListScreen::drawList(int baseY) {

    const int startY = baseY + HEADER_H + STATUS_H + LIST_PAD_Y;
    const int rows = visibleRows();

    for (int i = 0; i < rows; i++) {

        int idx = _scroll + i;
        if (idx >= _netCount)
            break;

        int y = startY + i * ROW_H;
        const char* ssid = _wifi.ssidAt(idx);

        bool selected = (i == _selected);

        if (selected) {
            _tft.fillRect(0, y, _tft.width(), ROW_H, theme().muted);
        }

        _tft.setTextSize(1);
        _tft.setTextColor(
            selected ? theme().bg : theme().textPrimary,
            selected ? theme().muted : theme().bg
        );

        _tft.setCursor(LIST_PAD_X, y + ROW_TEXT_Y_OFF);

        if (selected) {
            _tft.print("> ");
        } else {
            _tft.print("  ");
        }

        _tft.print(ssid);

        // ===== RSSI справа =====
        int rssi = WiFi.RSSI(idx);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", rssi);

        _tft.setCursor(_tft.width() - 28, y + ROW_TEXT_Y_OFF);
        _tft.setTextColor(
            selected ? theme().bg : theme().textSecondary,
            selected ? theme().muted : theme().bg
        );
        _tft.print(buf);

        if (i < rows - 1 && idx < _netCount - 1) {
            _tft.drawFastHLine(0, y + ROW_H - 1, _tft.width(), theme().muted);
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
    int usedH = HEADER_H + STATUS_H + LIST_PAD_Y + 4;
    int avail = _layout.contentH() - usedH;
    int rows  = avail / ROW_H;
    return (rows < 1) ? 1 : rows;
}

bool WifiListScreen::isConnectedSsid(const char* ssid) const {
    const char* cur = _wifi.currentSsid();
    return cur && ssid && strcmp(cur, ssid) == 0;
}