#include <TFT_eSPI.h>
#include <Timezone.h>

struct Day
{
    tm date;
    float minTemp;
    float maxTemp;
    String symbol_code;
    float precipitation_amount;
};

void getWeather(std::vector<Day> &days, Timezone *tz);

uint8_t *getWeatherIcon(const String &symbol_code);