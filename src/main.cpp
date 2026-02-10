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
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

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

void getCalendarEvents(std::vector<CalendarEvent> &events, String today)
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
            else if (line.startsWith("SUMMARY:"))
            {
                temp.summary = line.substring(8);
            }
            else if (line == "END:VEVENT")
            {
                if (temp.start >= today)
                {
                    events.push_back(temp);
                }
            }
        }

        std::sort(events.begin(), events.end(), [](const CalendarEvent &a, const CalendarEvent &b)
                  { return a.start < b.start; });

        Serial.println("Found " + String(events.size()) + " events");
        // print all events
        for (const auto &event : events)
        {
            Serial.println("Event: " + event.start + " - " + event.summary);
        }
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        Serial.println(":-(");
    }
}

void setup()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.begin(115200);

    prefs.begin("calendar");
    connectToWifi(wakeup_reason);

    struct tm timeinfo;
    setClock(&timeinfo);

    char timeBuffer[8];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d", &timeinfo);
    Serial.println(timeBuffer);

    std::vector<CalendarEvent> events;
    getCalendarEvents(events, timeBuffer);

    Epd epd;
    if (epd.Init() != 0)
    {
        Serial.print("e-Paper init failed");
        return;
    }
    epd.Clear();

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite frame = TFT_eSprite(&tft);

    frame.setColorDepth(4);

    frame.createSprite(EPD_WIDTH, EPD_HEIGHT);

    frame.setRotation(1);
    frame.rotation = 1;
    frame.setTextSize(1);                     // No size multiplier
    frame.fillSprite(INK_WHITE);              // Fill the screen with back colour
    frame.setTextColor(INK_BLACK, INK_WHITE); // Set text color to green and padding to back

    frame.setTextFont(8);
    frame.println(&timeinfo, "%Y-%m-%d %H:%M");

    frame.setFreeFont(&FreeSansNordic9pt7b);
    for (const auto &event : events)
    {
        // timeinfo
        tm timeinfo;
        timeinfo.tm_year = event.start.substring(0, 4).toInt() - 1900;
        timeinfo.tm_mon = event.start.substring(4, 6).toInt() - 1;
        timeinfo.tm_mday = event.start.substring(6, 8).toInt();
        timeinfo.tm_hour = event.start.substring(9, 11).toInt();
        timeinfo.tm_min = event.start.substring(11, 13).toInt();
        timeinfo.tm_sec = event.start.substring(13, 15).toInt();
        frame.setTextSize(1);
        frame.println(&timeinfo, "%Y-%m-%d %H:%M");
        frame.setTextSize(1);
        frame.println(event.summary);
        frame.println();
    }

    frame.readPixelValue(0, 0);

    epd.DisplayImage((uint8_t *)frame.frameBuffer(1));

    // epd.Displaypart(IMAGE_DATA, 250, 100, 240, 103, 0);
    // epd.Display_Part(IMAGE_DATA, 250, 300, 240, 103);

    epd.Sleep();
}

void loop()
{
    // put your main code here, to run repeatedly:
}