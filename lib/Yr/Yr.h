#include <TFT_eSPI.h>

struct Day
{
    String date;
    float minTemp;
    float maxTemp;
    String symbol_code;
};

void getWeather(std::vector<Day> &days);

uint8_t *getWeatherIcon(const String &symbol_code);