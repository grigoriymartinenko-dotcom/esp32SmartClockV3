#include "services/UiVersionService.h"

UiVersionService::UiVersionService() {
    for (uint8_t i = 0; i < N; i++) {
        _v[i] = 1;      // стартуем с 1 (не принципиально)
        _seen[i] = 0;   // чтобы первый changed() мог сработать при желании
    }
}
void UiVersionService::begin() {
    for (uint8_t i = 0; i < (uint8_t)UiChannel::COUNT; i++) {
        _v[i] = 0;
    }
}
void UiVersionService::bump(UiChannel ch) {
    uint8_t i = (uint8_t)ch;
    _v[i]++;
    if (_v[i] == 0) _v[i] = 1; // защита от переполнения в 0 (редко, но пусть будет)
}

uint32_t UiVersionService::version(UiChannel ch) const {
    return _v[(uint8_t)ch];
}

bool UiVersionService::changed(UiChannel ch) {
    uint8_t i = (uint8_t)ch;
    if (_seen[i] != _v[i]) {
        _seen[i] = _v[i];   // consume
        return true;
    }
    return false;
}