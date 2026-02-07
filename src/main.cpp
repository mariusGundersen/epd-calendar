#include <SPI.h>
#include "epd13in3b.h"
#include "imagedata.h"
#include <WiFiManager.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

Preferences prefs;

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

void setClock()
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

    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);
    log_d("Current time: %s", asctime(&timeinfo));
}

void setup()
{
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.begin(115200);

    prefs.begin("calendar");
    connectToWifi(wakeup_reason);
    setClock();

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
    frame.setTextSize(1);                     // No size multiplier
    frame.fillSprite(INK_WHITE);              // Fill the screen with back colour
    frame.setTextColor(INK_BLACK, INK_WHITE); // Set text color to green and padding to back

    frame.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2); // draw top left
    frame.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    frame.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    frame.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);

    frame.drawRoundRect(245, 95, 250, 108, 5, INK_BLACK);

    frame.drawRoundRect(245, 295, 250, 108, 5, INK_RED);

    frame.setCursor(10, 200);
    frame.setTextFont(1);
    frame.println("This is font 1");
    frame.setTextFont(2);
    frame.println("This is font 2");
    frame.setTextFont(4);
    frame.println("This is font 4");
    frame.setTextFont(6);
    frame.println("16:37.00");
    frame.setTextFont(7);
    frame.println("16:37.00");
    frame.setTextFont(8);
    frame.println("16:37.00");

    frame.fillRect(400, 100, 100, 100, INK_RED);
    frame.fillRect(400, 250, 100, 100, INK_DARK_RED);
    frame.fillRect(400, 400, 100, 100, INK_BLACK);
    frame.fillRect(550, 400, 100, 100, INK_DARK_GREY);
    frame.fillRect(700, 400, 100, 100, INK_GREY);
    frame.fillRect(850, 400, 100, 100, INK_LIGHT_GREY);

    frame.fillRect(550, 100, 100, 100, INK_PINK);
    frame.fillRect(700, 100, 100, 100, INK_LIGHT_PINK);

    for (char x = 0; x < 4; x++)
    {
        for (char y = 0; y < 4; y++)
        {
            frame.fillRect(8 + x * 16, 400 + y * 16, 16, 16, (x << 2) | y);
        }
    }

    epd.DisplayImage((uint8_t *)frame.frameBuffer(1));

    // epd.Displaypart(IMAGE_DATA, 250, 100, 240, 103, 0);
    // epd.Display_Part(IMAGE_DATA, 250, 300, 240, 103);

    epd.Sleep();
}

void loop()
{
    // put your main code here, to run repeatedly:
}