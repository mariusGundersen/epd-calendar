#include <Arduino.h>
#include <SPI.h>
#include "epd13in3b.h"
#include "imagedata.h"
#include <WiFiManager.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <Preferences.h>
#include <FreeSansNordic9pt7b.h>
#include <FreeSansBoldNordic9pt7b.h>
#include <FreeSansBoldNordic12pt7b.h>
#include <SmallTemp.h>
#include <Timezone.h>
#include "Yr.h"
#include "Battery.h"

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

enum struct SleepDuration
{
    untilTomorrow,
    untilNextHour,
    fiveMinutes
};

Preferences prefs;

const char *googleRootCACert = "-----BEGIN CERTIFICATE-----\n"
                               "MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n"
                               "CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
                               "MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n"
                               "MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
                               "Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n"
                               "A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n"
                               "27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n"
                               "Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n"
                               "TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n"
                               "qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n"
                               "szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n"
                               "Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n"
                               "MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n"
                               "wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n"
                               "aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n"
                               "VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n"
                               "AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n"
                               "FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n"
                               "C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n"
                               "QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n"
                               "h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n"
                               "7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n"
                               "ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n"
                               "MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n"
                               "Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n"
                               "6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n"
                               "0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n"
                               "2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n"
                               "bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
                               "-----END CERTIFICATE-----";

void connectToWifi(esp_sleep_wakeup_cause_t wakeup_reason, bool reset = false)
{
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    WiFiManagerParameter ical_url("ical_url", "iCal URL", prefs.getString("ical_url").c_str(), 256);
    wm.addParameter(&ical_url);

    if (reset)
    {
        wm.resetSettings();
    }

    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
        // try to connect, if fails, show message screen and go back to sleep
        wm.setConnectTimeout(0);
        wm.setEnableConfigPortal(false);
    }
    else
    {
        // try to connect, if fails, show config portal for 2 minutes, then show message screen and go back to sleep
        wm.setConfigPortalTimeout(120);
        wm.setEnableConfigPortal(true);
        wm.setCaptivePortalEnable(true);

        wm.setSaveConfigCallback([&]()
                                 { prefs.putString("ical_url", ical_url.getValue()); });
    }

    if (wm.autoConnect("calendar", "password"))
    {
        // counter = 0;
    }
    else
    {
        /*counter++;
        if (counter > 5 || wakeup_reason != ESP_SLEEP_WAKEUP_TIMER)
        {
            gfx->fillScreen(RGB_WHITE);
            gfx->setFont(&FreeSans24pt7b);
            gfx->setTextColor(RGB_BLACK);
            gfx->printCentredText("Press Reset button");
            gfx->updateDisplay();

            // make sure it refreshes the screen when it reconnects
            prefs.remove("ETag");
            enterDeepSleep(SleepDuration::untilTomorrow);
        }
        else
        {
            // now sleep for 1 hour then retry
            enterDeepSleep(SleepDuration::untilNextHour);
        }*/
    }
}

void setClock(tm *timeinfo)
{
    // TODO: replace with ezTime
    configTime(0, 0, "pool.ntp.org");
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");

    log_d("Waiting for NTP time sync: ");
    time_t nowSecs = time(nullptr);
    while (nowSecs < 10)
    {
        delay(500);
        yield();
        nowSecs = time(nullptr);
    }

    gmtime_r(&nowSecs, timeinfo);
    log_d("Current time: %s\n", asctime(timeinfo));
}

struct CalendarEvent
{
    bool fullDay;
    String start;
    String end;
    String summary;
};

void getCalendarEvents(std::vector<CalendarEvent> &events, String notBefore, String notAfter)
{
    NetworkClientSecure client;

    client.setCACert(googleRootCACert);
    String urls = prefs.getString("ical_url");
    urls.concat(' ');

    HTTPClient http;

    int start = 0;
    int separator = urls.indexOf(' ');

    while (separator != -1)
    {

        String url = urls.substring(start, separator);

        http.begin(client, url);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            log_d("HTTP response code: %d\n", httpResponseCode);
            NetworkClient &stream = http.getStream();
            CalendarEvent temp;
            String line;
            while (http.connected())
            {
                line = stream.readStringUntil('\n');
                if (line == "" && !stream.available())
                {
                    log_d("End of stream reached\n");
                    break;
                }
                line.trim();
                if (line == "BEGIN:VEVENT")
                {
                    temp = CalendarEvent();
                }
                else if (line.startsWith("DTSTART:"))
                {
                    temp.start = line.substring(8);
                }
                else if (line.startsWith("DTSTART;VALUE=DATE:"))
                {
                    temp.fullDay = true;
                    temp.start = line.substring(19);
                }
                else if (line.startsWith("DTEND:"))
                {
                    temp.end = line.substring(6);
                }
                else if (line.startsWith("DTEND;VALUE=DATE:"))
                {
                    temp.fullDay = true;
                    temp.end = line.substring(17);
                }
                else if (line.startsWith("SUMMARY:"))
                {
                    temp.summary = line.substring(8);
                }
                else if (line == "END:VEVENT")
                {
                    if (temp.start < notAfter && temp.end > notBefore)
                    {
                        events.push_back(temp);
                    }
                }
            }
        }
        else
        {
            log_d("Error code: %d\n", httpResponseCode);
        }

        start = separator + 1;
        separator = urls.indexOf(' ', start);
    }

    std::sort(events.begin(), events.end(), [](const CalendarEvent &a, const CalendarEvent &b)
              { return a.start < b.start; });

    log_d("Between %s and %s\n", notBefore.c_str(), notAfter.c_str());
    log_d("Found %d events", events.size());
    // print all events
    for (const auto &event : events)
    {
        log_d("Event: %s - %s - %s\n", event.start.c_str(), event.summary.c_str(), event.end.c_str());
    }
}

void enterDeepSleep(SleepDuration sleepDuration)
{
    time_t nowSecs = time(nullptr);
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);

    int hoursToSleep = sleepDuration == SleepDuration::untilTomorrow ? 3 - timeinfo.tm_hour : 0;
    if (sleepDuration == SleepDuration::untilTomorrow && hoursToSleep < 1)
    {
        hoursToSleep += 24;
    }

    int minutesToSleep = sleepDuration == SleepDuration::fiveMinutes ? 5 : 59 - timeinfo.tm_min;
    if (minutesToSleep < 5)
    {
        minutesToSleep += 60;
    }

    int secondsToSleep = 60 - timeinfo.tm_sec;
    int sleepTime = (hoursToSleep * 60 + minutesToSleep) * 60 + secondsToSleep;

    esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);
    log_d("Setup ESP32 to sleep for %d Seconds\n", sleepTime);

    // This didn't work, need pull-up resistor
    // pinMode(GPIO_NUM_0, INPUT_PULLUP);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    // esp_sleep_enable_ext1_wakeup(1 << GPIO_NUM_0, ESP_EXT1_WAKEUP_ANY_LOW);

    log_d("Going to sleep now\n");
    // if (Serial)
    // Serial.flush();

    sleep(10); // give some time for the message to be sent before sleeping, otherwise it might not be sent at all

    esp_deep_sleep_start();
}

void toLocalTime(const String &time, tm *timeinfo, Timezone *tz)
{
    timeinfo->tm_year = time.substring(0, 4).toInt() - 1900;
    timeinfo->tm_mon = time.substring(4, 6).toInt() - 1;
    timeinfo->tm_mday = time.substring(6, 8).toInt();
    if (time.length() > 8)
    {
        timeinfo->tm_hour = time.substring(9, 11).toInt();
        timeinfo->tm_min = time.substring(11, 13).toInt();
        timeinfo->tm_sec = time.substring(13, 15).toInt();
        timeinfo->tm_isdst = -1; // auto-detect DST
        time_t t = mktime(timeinfo);
        time_t localTime = tz->toLocal(t);
        localtime_r(&localTime, timeinfo);
    }
    else
    {
        timeinfo->tm_isdst = -1; // auto-detect DST
        time_t t = mktime(timeinfo);
        time_t localTime = tz->toLocal(t);
        localtime_r(&localTime, timeinfo);
        timeinfo->tm_hour = 0;
        timeinfo->tm_min = 0;
        timeinfo->tm_sec = 0;
        log_d("Local time\n%s\n", asctime(timeinfo));
    }
}

const char *months[] = {
    "Januar",
    "Februar",
    "Mars",
    "April",
    "Mai",
    "Juni",
    "Juli",
    "August",
    "September",
    "Oktober",
    "November",
    "Desember"};

const char *weekdays[] = {
    "Søndag",
    "Mandag",
    "Tirsdag",
    "Onsdag",
    "Torsdag",
    "Fredag",
    "Lørdag"};

void drawMeteogram(TFT_eSprite frame, WeatherRange weatherRange, std::vector<Hour> &weatherHours)
{
    log_d("Weather range: minTemp=%f, maxTemp=%f, maxPrecipitation=%f\n", weatherRange.minTemp, weatherRange.maxTemp, weatherRange.maxPrecipitation);

    int yOffset = 20;
    int yPaddingBottom = 10;
    int height = frame.height() - yOffset - yPaddingBottom;
    int width = frame.width();

    float minTemp = min(0.0f, weatherRange.minTemp);
    float maxTemp = max(4.0f, weatherRange.maxTemp);
    float tempMultiplier = (height) / (maxTemp - minTemp);
    float zeroY = yOffset + (maxTemp * tempMultiplier);
    float maxPrecipitation = max(weatherRange.maxPrecipitation, 4.0f);
    float precipitationMultiplier = (maxTemp * tempMultiplier) / maxPrecipitation;
    int hourWidth = 10;

    log_d("Temp multiplier: %f\n", tempMultiplier);
    log_d("Precipitation multiplier: %f\n", precipitationMultiplier);
    frame.drawFastHLine(10, zeroY, width - 10, INK_GREY);

    int previousTemp = weatherHours.at(0).temperature * tempMultiplier;
    int previousHour = weatherHours.at(0).hourOffset;

    // vertical now line
    frame.drawFastVLine(previousHour * hourWidth, yOffset, height, INK_BLACK);

    frame.setFreeFont(&SmallTemp);

    // Max temperature
    char temp[10];
    sprintf(temp, "%.0f°C ", maxTemp);
    frame.setTextDatum(TR_DATUM);
    frame.drawString(temp, previousHour * hourWidth, yOffset);

    // Min temperature
    sprintf(temp, "%.0f°C ", minTemp);
    frame.setTextDatum(BR_DATUM);
    frame.drawString(temp, previousHour * hourWidth, yOffset + height);

    for (int h = 1; h < 24 * 4; h++)
    {
        if (h % 24 == 0)
        {
            frame.drawFastVLine(h * hourWidth, zeroY - 8, 16, INK_BLACK);
        }
        else if (h % 6 == 0)
        {
            frame.drawFastVLine(h * hourWidth, zeroY - 4, 4, INK_BLACK);
            frame.setTextDatum(TC_DATUM);
            frame.drawString(String(h % 24), h * hourWidth, zeroY + 2);
        }
    }

    for (int h = 0; h < weatherHours.size(); h++)
    {
        Hour hour = weatherHours.at(h);
        int hourX = (hour.hourOffset) * hourWidth;

        if (h < weatherHours.size() - 1)
        {
            Hour nextHour = weatherHours.at(h + 1);
            int width = (nextHour.hourOffset - hour.hourOffset) * hourWidth;
            int height = hour.precipitation_amount * precipitationMultiplier;
            if (height > 0)
            {
                frame.fillRect(hourX, zeroY - height + 1, width, height, INK_LIGHT_GREY);
                frame.drawRect(hourX, zeroY - height + 1, width, height, INK_BLACK);
            }
        }

        int tempY = hour.temperature * tempMultiplier;
        frame.drawLine(previousHour * hourWidth, zeroY - previousTemp, hourX, zeroY - tempY, INK_RED);
        frame.drawLine(previousHour * hourWidth, zeroY - previousTemp - 1, hourX, zeroY - tempY - 1, INK_RED);

        if (hour.hourOffset % 2 == 1)
        {
            frame.drawXBitmap(hourX - 8, zeroY - tempY - 20, getWeatherIcon16(hour.symbol), 16, 16, INK_BLACK);
        }

        previousTemp = tempY;

        previousHour = hour.hourOffset;
    }
}

void setup()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.begin();

    Epd epd;
    if (epd.Init() != 0)
    {
        log_d("e-Paper init failed\n");
        return;
    }
    epd.Clear();

    prefs.begin("calendar");
    connectToWifi(wakeup_reason);

    struct tm timeinfo;
    setClock(&timeinfo);

    char today[9], tomorrow[9];
    strftime(today, sizeof(today), "%Y%m%d", &timeinfo);
    timeinfo.tm_mday += 4;
    mktime(&timeinfo);
    strftime(tomorrow, sizeof(tomorrow), "%Y%m%d", &timeinfo);
    log_d("Today: %s\n", today);
    log_d("Tomorrow: %s\n", tomorrow);

    getLocalTime(&timeinfo);

    TimeChangeRule tcr("CEST", Last, Sun, Mar, 2, 120); // Central European Summer Time = UTC + 2 hours
    TimeChangeRule tcr2("CET", Last, Sun, Oct, 3, 60);  // Central European Standard Time = UTC + 1 hour
    Timezone europeOslo(tcr, tcr2);

    std::vector<CalendarEvent> events;
    getCalendarEvents(events, today, tomorrow);

    std::vector<Day> weatherDays;
    std::vector<Hour> weatherHours;
    WeatherRange weatherRange = getWeather(weatherDays, weatherHours, &europeOslo);

    // turn off wifi to save power, we don't need it anymore
    WiFi.mode(WIFI_OFF);

    Battery battery = getBatteryStatus();

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite frame = TFT_eSprite(&tft);

    frame.setColorDepth(4);

    frame.createSprite(EPD_WIDTH, EPD_HEIGHT);

    frame.setRotation(1);
    frame.rotation = 1;
    frame.setTextSize(1);                     // No size multiplier
    frame.fillSprite(INK_WHITE);              // Fill the screen with back colour
    frame.setTextColor(INK_BLACK, INK_WHITE); // Set text color to green and padding to back

    frame.setFreeFont(&FreeSerifBoldItalic24pt7b);
    frame.setTextSize(1);
    frame.println();
    frame.printf("%d. %s %d", timeinfo.tm_mday, months[timeinfo.tm_mon], timeinfo.tm_year + 1900);
    frame.setFreeFont(&FreeSansNordic9pt7b);

    int y = frame.getCursorY() + 10;
    frame.setViewport(0, y - 10, EPD_WIDTH, 70);

    drawMeteogram(frame, weatherRange, weatherHours);

    // height of box is 60px and 10px padding at bottom
    y += 10 + 60;

    const int padding = 4;
    for (int day = 0; day < 4; day++)
    {
        frame.setViewport(day * EPD_WIDTH / 4 + padding, y, EPD_WIDTH / 4 - padding * 2, EPD_HEIGHT - y);
        // Day of week
        frame.setFreeFont(&FreeSansBoldNordic12pt7b);
        frame.setCursor(0, 0);
        frame.println();
        frame.setTextWrap(true);
        frame.println(weekdays[timeinfo.tm_wday]);

        // weather info
        frame.setFreeFont(&FreeSansBoldNordic9pt7b);
        frame.setCursor(0, frame.getCursorY() + 4);
        frame.fillRoundRect(0, frame.getCursorY() - 24, EPD_WIDTH / 4 - padding * 2, 36, 8, INK_LIGHT_GREY);
        frame.setTextPadding(4);
        for (const auto &weatherDay : weatherDays)
        {
            if (timeinfo.tm_mday == weatherDay.date.tm_mday)
            {
                uint8_t *icon = getWeatherIcon32(weatherDay.symbol_code);
                frame.drawXBitmap(4, frame.getCursorY() - 22, icon, 32, 32, INK_BLACK);
                frame.setCursor(40, frame.getCursorY());
                int minPrecision = weatherDay.minTemp < 10 ? 1 : 0;
                int maxPrecision = weatherDay.maxTemp < 10 ? 1 : 0;
                frame.printf("%.*f°C / %.*f°C", minPrecision, weatherDay.minTemp, maxPrecision, weatherDay.maxTemp);
                int precipitationPrecision = weatherDay.precipitation_amount < 10 ? 1 : 0;
                frame.printf("   %.*f mm", precipitationPrecision, weatherDay.precipitation_amount);
                // TODO: remove this weatherDay since it has already been consumed

                break;
            }
        }
        frame.setCursor(0, frame.getCursorY() + 32);
        frame.setTextPadding(0);

        for (const auto &event : events)
        {
            tm start;
            toLocalTime(event.start, &start, &europeOslo);
            tm end;
            toLocalTime(event.end, &end, &europeOslo);

            if (start.tm_mday != timeinfo.tm_mday)
            {
                continue;
            }

            frame.setFreeFont(&FreeSansBoldNordic9pt7b);
            if (event.fullDay)
            {
                frame.print("Hele dagen");
            }
            else
            {
                frame.print(&start, "%H:%M");
            }
            if (start.tm_mon != end.tm_mon)
            {
                frame.print(" » ");
                frame.print(&end, "%d. %B");
            }
            else if (start.tm_mday != end.tm_mday)
            {
                frame.print(" » ");
                frame.print(weekdays[end.tm_wday]);
            }
            else
            {
                frame.print(" - ");
                frame.print(&end, "%H:%M");
            }
            frame.println();
            frame.setFreeFont(&FreeSansNordic9pt7b);
            frame.println(event.summary);
            frame.println();
        }

        frame.println();
        timeinfo.tm_mday += 1;
        mktime(&timeinfo);
    }

    frame.setViewport(0, 0, EPD_WIDTH, EPD_HEIGHT);

    frame.setTextDatum(BR_DATUM);

    char buffer[64];
    strftime(buffer, 64, "%H:%M %d/%m", &timeinfo);

    frame.setTextFont(1);
    frame.drawString("Sist oppdatert: " + String(buffer) + " (" + String(battery.cellVoltage) + "V)", EPD_WIDTH - 5, EPD_HEIGHT - 5);

    // make sure we are ready again
    epd.ReadBusy();

    epd.DisplayImage((uint8_t *)frame.frameBuffer(1), 180);

    epd.Sleep();

    enterDeepSleep(SleepDuration::untilTomorrow);

    // esp_light_sleep_start();
}

void loop()
{
    // put your main code here, to run repeatedly:
}