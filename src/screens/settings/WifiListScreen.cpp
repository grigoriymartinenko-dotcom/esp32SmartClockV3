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
    WifiService& wifi,
    PreferencesService& prefs,
    ButtonBar& buttonBar
)
    : Screen(theme)
    , _tft(tft)
    , _layout(layout)
    , _ui(ui)
    , _wifi(wifi)
    , _prefs(prefs)
    , _buttons(buttonBar)
{
}

// ============================================================================
// lifecycle
// ============================================================================
void WifiListScreen::begin() {

    _scroll   = 0;
    _selected = 0;
    _netCount = _wifi.networksCount();

    _state = _wifi.isScanning()
        ? State::SCANNING
        : State::READY;

    _lastWifiVer   = _ui.version(UiChannel::WIFI);
    _lastScreenVer = _ui.version(UiChannel::SCREEN);

    updateButtonBarContext();
}

void WifiListScreen::update() {

    // --- determine state from WifiService ---
    if (_wifi.isScanning()) {
        _state = State::SCANNING;
    } else {
        _netCount = _wifi.networksCount();

        switch (_wifi.state()) {
            case WifiService::State::ONLINE:
                _state = State::CONNECTED;
                break;

            case WifiService::State::CONNECTING:
                _state = State::RECONNECTING;
                break;

            default:
                _state = State::READY;
                break;
        }
    }

    if (connectionModelDirty()) {
        updateButtonBarContext();
        redrawAll();
    }
}

// ============================================================================
// drawing
// ============================================================================
void WifiListScreen::redrawAll() {

    const Theme& th = theme();
    const int topY = 0;
    const int botY = _layout.buttonBarY();

    _tft.fillRect(0, topY, _tft.width(), botY, th.bg);

    drawHeader(0);
    drawConnectionStatus(26);
    drawSeparator(46);

    if (_state == State::SCANNING) {
        drawScanning(56);
    } else {
        drawList(56);
    }
}

void WifiListScreen::drawHeader(int baseY) {

    const Theme& th = theme();

    _tft.setTextSize(2);
    _tft.setTextColor(th.textPrimary, th.bg);
    _tft.setCursor(18, baseY + 6);
    _tft.print("Wi-Fi");
    _tft.setTextSize(1);
}

void WifiListScreen::drawConnectionStatus(int baseY) {

    const Theme& th = theme();
    _tft.setCursor(8, baseY);

    if (_state == State::CONNECTED) {
        _tft.setTextColor(th.textPrimary, th.bg);
        _tft.print("Connected");
    } else if (_state == State::RECONNECTING) {
        _tft.setTextColor(th.warn, th.bg);
        _tft.print("Reconnecting...");
    } else {
        _tft.setTextColor(th.muted, th.bg);
        _tft.print("Not connected");
    }
}

void WifiListScreen::drawSeparator(int y) {

    const Theme& th = theme();
    _tft.drawFastHLine(0, y, _tft.width(), th.textSecondary);
}

void WifiListScreen::drawScanning(int baseY) {

    const Theme& th = theme();
    _tft.setCursor(12, baseY + 12);
    _tft.setTextColor(th.muted, th.bg);
    _tft.print("Scanning...");
}

void WifiListScreen::drawList(int baseY) {

    const Theme& th = theme();
    const int rowH = 16;
    const int visible = visibleRows();

    for (int i = 0; i < visible; i++) {

        const int idx = _scroll + i;
        const int y   = baseY + i * rowH;

        if (idx > _netCount) break;

        const bool isRescan = (idx == _netCount);
        const bool sel = (idx == _selected);

        _tft.setCursor(8, y + 4);
        _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
        _tft.print(sel ? "> " : "  ");

        if (isRescan) {
            _tft.print("|--- Rescan ---|");
        } else {
            _tft.print(_wifi.ssidAt(idx));
        }
    }
}

// ============================================================================
// helpers
// ============================================================================
int WifiListScreen::visibleRows() const {

    const int top = 56;
    const int bottom = _layout.buttonBarY();
    return (bottom - top) / 16;
}

bool WifiListScreen::isConnectedSsid(const char* ssid) const {

    return (_wifi.state() == WifiService::State::ONLINE)
        && ssid
        && strcmp(ssid, _wifi.currentSsid()) == 0;
}

bool WifiListScreen::connectionModelDirty() {

    const WifiService::State s = _wifi.state();

    if (s != _lastConnState) {
        _lastConnState = s;
        return true;
    }

    return false;
}

// ============================================================================
// ButtonBar context
// ============================================================================
void WifiListScreen::updateButtonBarContext() {

    _buttons.setHighlight(false, false, false, false);

    switch (_state) {

        case State::SCANNING:
        case State::CONNECTED:
        case State::RECONNECTING:
            _buttons.setActions(false, false, false, true);
            _buttons.setLabels(nullptr, nullptr, nullptr, "BACK");
            break;

        case State::READY: {
            const bool onRescan = (_selected == _netCount);

            _buttons.setActions(true, true, true, true);

            if (onRescan) {
                _buttons.setLabels("<", "RESCAN", ">", "BACK");
                _buttons.setHighlight(false, true, false, false);
            } else {
                _buttons.setLabels("<", "SELECT", ">", "BACK");
            }
            break;
        }
    }

    _buttons.markDirty();
}

// ============================================================================
// input
// ============================================================================
void WifiListScreen::onShortLeft() {

    if (_state != State::READY) return;

    if (_selected > 0) {
        _selected--;
        if (_selected < _scroll)
            _scroll = _selected;
        updateButtonBarContext();
    }
}

void WifiListScreen::onShortRight() {

    if (_state != State::READY) return;

    const int maxIdx = _netCount;

    if (_selected < maxIdx) {
        _selected++;
        if (_selected >= _scroll + visibleRows())
            _scroll = _selected - visibleRows() + 1;
        updateButtonBarContext();
    }
}

void WifiListScreen::onShortOk() {

    if (_state != State::READY) return;

    // RESCAN
    if (_selected == _netCount) {
        _wifi.startScan();
        _state = State::SCANNING;
        updateButtonBarContext();
        return;
    }

    // SELECT
    const char* ssid = _wifi.ssidAt(_selected);
    if (!ssid) return;

    _wifi.connect(ssid, nullptr);
    _state = State::RECONNECTING;
    updateButtonBarContext();
}

void WifiListScreen::onShortBack() {
    // handled by ScreenManager
}