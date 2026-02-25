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
#include <FreeSansBold9pt7b.h>
#include <Timezone.h>
#include "Yr.h"

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
    log_d("Current time: %s", asctime(timeinfo));
}

struct CalendarEvent
{
    String start;
    String end;
    String summary;
};

void getCalendarEvents(std::vector<CalendarEvent> &events, String notBefore, String notAfter)
{
    NetworkClientSecure client;

    client.setCACert(googleRootCACert);
    String url = prefs.getString("ical_url");
    HTTPClient http;
    http.begin(client, url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
        Serial.print("HTTP ");
        Serial.println(httpResponseCode);
        NetworkClient &stream = http.getStream();
        CalendarEvent temp;
        String line;
        while (http.connected())
        {
            line = stream.readStringUntil('\n');
            if (line == "" && !stream.available())
            {
                Serial.println("End of stream reached");
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
                temp.start = line.substring(19);
            }
            else if (line.startsWith("DTEND:"))
            {
                temp.end = line.substring(6);
            }
            else if (line.startsWith("DTEND;VALUE=DATE:"))
            {
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

        std::sort(events.begin(), events.end(), [](const CalendarEvent &a, const CalendarEvent &b)
                  { return a.start < b.start; });

        Serial.println("Between " + notBefore + " and " + notAfter);
        Serial.println("Found " + String(events.size()) + " events");
        // print all events
        for (const auto &event : events)
        {
            Serial.println("Event: " + event.start + " - " + event.summary + " - " + event.end);
        }
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        Serial.println(":-(");
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

    esp_deep_sleep_start();
}

void toLocalTime(const String &time, tm *timeinfo, Timezone *tz)
{
    timeinfo->tm_year = time.substring(0, 4).toInt() - 1900;
    timeinfo->tm_mon = time.substring(4, 6).toInt() - 1;
    timeinfo->tm_mday = time.substring(6, 8).toInt();
    timeinfo->tm_hour = time.substring(9, 11).toInt();
    timeinfo->tm_min = time.substring(11, 13).toInt();
    timeinfo->tm_sec = time.substring(13, 15).toInt();
    timeinfo->tm_isdst = -1; // auto-detect DST
    time_t t = mktime(timeinfo);
    time_t localTime = tz->toLocal(t);
    localtime_r(&localTime, timeinfo);
}

void setup()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.begin(115200);

    Epd epd;
    if (epd.Init() != 0)
    {
        Serial.print("e-Paper init failed");
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
    Serial.println(today);
    Serial.println(tomorrow);

    getLocalTime(&timeinfo);

    TimeChangeRule tcr("CEST", Last, Sun, Mar, 2, 120); // Central European Summer Time = UTC + 2 hours
    TimeChangeRule tcr2("CET", Last, Sun, Oct, 3, 60);  // Central European Standard Time = UTC + 1 hour
    Timezone europeOslo(tcr, tcr2);

    std::vector<CalendarEvent> events;
    getCalendarEvents(events, today, tomorrow);

    std::vector<Day> weatherDays;
    getWeather(weatherDays);

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
    frame.println(&timeinfo, "%A %d. %B %Y %H:%M");
    frame.setFreeFont(&FreeSansNordic9pt7b);
    int y = frame.getCursorY();
    frame.drawWideLine(10, y, EPD_WIDTH - 10, y, 6, INK_RED);
    const int padding = 4;
    for (int day = 0; day < 4; day++)
    {
        frame.setViewport(day * EPD_WIDTH / 4 + padding, y, EPD_WIDTH / 4 - padding * 2, EPD_HEIGHT - y);
        // Day of week
        frame.setFreeFont(&FreeSansBold12pt7b);
        frame.setCursor(0, 0);
        frame.println();
        frame.setTextWrap(true);
        frame.println(&timeinfo, "%A");

        // weather info
        frame.setFreeFont(&FreeSansBold_extended9pt7b);
        frame.fillRoundRect(0, frame.getCursorY() - 24, EPD_WIDTH / 4 - padding * 2, 36, 8, INK_LIGHT_GREY);
        frame.setTextPadding(4);
        for (const auto &weatherDay : weatherDays)
        {
            if (timeinfo.tm_mday == weatherDay.date.substring(8, 10).toInt())
            {
                uint8_t *icon = getWeatherIcon(weatherDay.symbol_code);
                frame.drawXBitmap(4, frame.getCursorY() - 22, icon, 32, 32, INK_BLACK);
                frame.setCursor(40, frame.getCursorY());
                frame.printf("%.1f°C - %.1f°C", weatherDay.minTemp, weatherDay.maxTemp);
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

            frame.setFreeFont(&FreeSansBold_extended9pt7b);
            frame.print(&start, "%H:%M");
            frame.print(" - ");
            frame.println(&end, "%H:%M");
            frame.setFreeFont(&FreeSansNordic9pt7b);
            frame.println(event.summary);
            frame.println();
        }

        frame.println();
        timeinfo.tm_mday += 1;
        mktime(&timeinfo);
    }

    // make sure we are ready again
    epd.ReadBusy();

    epd.DisplayImage((uint8_t *)frame.frameBuffer(1), 180);

    epd.Sleep();

    enterDeepSleep(SleepDuration::untilTomorrow);
}

void loop()
{
    // put your main code here, to run repeatedly:
}