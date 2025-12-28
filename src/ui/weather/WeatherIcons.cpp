#include "ui/weather/WeatherIcons.h"

// 16x16 bitmap icons
#include "ui/weather/icons/clear_day_16.h"
#include "ui/weather/icons/clear_night_16.h"
#include "ui/weather/icons/clouds_16.h"
#include "ui/weather/icons/rain_16.h"
#include "ui/weather/icons/thunder_16.h"
#include "ui/weather/icons/snow_16.h"
#include "ui/weather/icons/fog_16.h"

/*
 * OpenWeather mapping:
 * 200–232 thunder
 * 300–531 rain
 * 600–622 snow
 * 701–781 fog
 * 800 clear
 * 801–804 clouds
 */

WeatherIcon getWeatherIcon(int weatherCode, bool night) {

    if (weatherCode >= 200 && weatherCode <= 232) {
        return { icon_thunder_16, 16, 16 };
    }

    if (weatherCode >= 300 && weatherCode <= 531) {
        return { icon_rain_16, 16, 16 };
    }

    if (weatherCode >= 600 && weatherCode <= 622) {
        return { icon_snow_16, 16, 16 };
    }

    if (weatherCode >= 701 && weatherCode <= 781) {
        return { icon_fog_16, 16, 16 };
    }

    if (weatherCode == 800) {
        return night
            ? WeatherIcon{ icon_clear_night_16, 16, 16 }
            : WeatherIcon{ icon_clear_day_16,   16, 16 };
    }

    if (weatherCode >= 801 && weatherCode <= 804) {
        return { icon_clouds_16, 16, 16 };
    }

    return { icon_clouds_16, 16, 16 };
}