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

struct Hour
{
    int hourOffset;
    float temperature;
    float precipitation_amount;
    String symbol;
};

struct WeatherRange
{
    float minTemp;
    float maxTemp;
    float maxPrecipitation;
};

WeatherRange getWeather(std::vector<Day> &days, std::vector<Hour> &hours, Timezone *tz);

uint8_t *getWeatherIcon(const String &symbol_code);