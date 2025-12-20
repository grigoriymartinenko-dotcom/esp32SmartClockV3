#include "services/UiVersionService.h"

void UiVersionService::begin() {
    for (uint8_t i = 0; i < (uint8_t)UiChannel::COUNT; i++) {
        _versions[i] = 0;
    }
}

void UiVersionService::bump(UiChannel ch) {
    _versions[(uint8_t)ch]++;
}

uint32_t UiVersionService::version(UiChannel ch) const {
    return _versions[(uint8_t)ch];
}