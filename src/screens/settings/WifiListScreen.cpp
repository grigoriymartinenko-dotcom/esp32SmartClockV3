#include "screens/settings/WifiListScreen.h"
#include <Adafruit_GFX.h>

// ============================================================================
// UX constants
// ============================================================================
static constexpr uint32_t PROGRESS_INTERVAL_MS = 300;
static constexpr uint32_t INLINE_TIMEOUT_MS   = 1500;
static constexpr int      PROGRESS_BLOCKS     = 8;

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

    _state = (_wifi.scanState() == WifiService::ScanState::SCANNING)
        ? State::SCANNING
        : State::READY;

    _lastWifiVer   = _ui.version(UiChannel::WIFI);
    _lastScreenVer = _ui.version(UiChannel::SCREEN);

    updateButtonBarContext();
    redrawAll();
}

void WifiListScreen::update() {

    // ------------------------------------------------------------
    // Scan lifecycle → FSM
    // ------------------------------------------------------------
    switch (_wifi.scanState()) {

        case WifiService::ScanState::SCANNING:
            _state = State::SCANNING;
            break;

        case WifiService::ScanState::DONE:
        case WifiService::ScanState::FAILED:
        case WifiService::ScanState::IDLE:
        default:
            _state = State::READY;
            break;
    }

    _netCount = _wifi.networksCount();

    if (_ui.changed(UiChannel::WIFI) || _ui.changed(UiChannel::SCREEN)) {
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

    switch (_wifi.state()) {

        case WifiService::State::ONLINE:
            _tft.setTextColor(th.textPrimary, th.bg);
            _tft.print("Connected");
            break;

        case WifiService::State::CONNECTING:
            _tft.setTextColor(th.warn, th.bg);
            _tft.print("Connecting...");
            break;

        default:
            _tft.setTextColor(th.muted, th.bg);
            _tft.print("Not connected");
            break;
    }
}

void WifiListScreen::drawSeparator(int y) {

    const Theme& th = theme();
    _tft.drawFastHLine(0, y, _tft.width(), th.textSecondary);
}

// ---------------------------------------------------------------------------
// SCANNING — mini progress
// ---------------------------------------------------------------------------
void WifiListScreen::drawScanning(int baseY) {

    const Theme& th = theme();
    const uint32_t step =
        (millis() / PROGRESS_INTERVAL_MS) % (PROGRESS_BLOCKS + 1);

    _tft.setCursor(12, baseY + 10);
    _tft.setTextColor(th.muted, th.bg);
    _tft.print("Scanning");

    _tft.setCursor(12, baseY + 26);
    _tft.print("[");

    for (int i = 0; i < PROGRESS_BLOCKS; ++i) {
        _tft.print(i < (int)step ? '#' : ' ');
    }
    _tft.print("]");
}

// ---------------------------------------------------------------------------
// LIST
// ---------------------------------------------------------------------------
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

        if (!isRescan && isConnectedSsid(_wifi.ssidAt(idx))) {
            _tft.setTextColor(th.textPrimary, th.bg);
            _tft.print("✓ ");
        } else {
            _tft.setTextColor(sel ? th.select : th.textPrimary, th.bg);
            _tft.print(sel ? "> " : "  ");
        }

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

// ============================================================================
// ButtonBar context
// ============================================================================
void WifiListScreen::updateButtonBarContext() {

    _buttons.setHighlight(false, false, false, false);

    switch (_state) {

        case State::SCANNING:
            _buttons.setActions(false, false, false, true);
            _buttons.setLabels(nullptr, nullptr, nullptr, "BACK");
            break;

        case State::READY: {
            const bool onRescan = (_selected == _netCount);
            const bool onConnected =
                !onRescan && isConnectedSsid(_wifi.ssidAt(_selected));

            _buttons.setActions(true, true, true, true);

            if (onRescan) {
                _buttons.setLabels("<", "RESCAN", ">", "BACK");
                _buttons.setHighlight(false, true, false, false);
            } else if (onConnected) {
                _buttons.setLabels("<", "OK", ">", "BACK");
            } else {
                _buttons.setLabels("<", "SELECT", ">", "BACK");
            }
            break;
        }

        default:
            break;
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
        redrawAll();
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
        redrawAll();
    }
}

void WifiListScreen::onShortOk() {

    if (_state != State::READY) return;

    // RESCAN
    if (_selected == _netCount) {
        _wifi.startScan();
        _state = State::SCANNING;
        updateButtonBarContext();
        redrawAll();
        return;
    }

    const char* ssid = _wifi.ssidAt(_selected);
    if (!ssid || !ssid[0]) return;

    // Already connected → UX feedback only
    if (isConnectedSsid(ssid)) {
        _tft.setCursor(8, _layout.buttonBarY() - 14);
        _tft.setTextColor(theme().muted, theme().bg);
        _tft.print("Already connected");
        return;
    }

    // CONNECT
    _wifi.connect(ssid, nullptr);
    updateButtonBarContext();
}

void WifiListScreen::onShortBack() {
    // handled by ScreenManager
}