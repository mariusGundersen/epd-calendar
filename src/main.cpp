/**
 *  @filename   :   epd7in5b_V2-demo.ino
 *  @brief      :   7.5inch b V2 e-paper display demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare      Nov 30 2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include "epd13in3b.h"
#include "imagedata.h"
#include <TFT_eSPI.h>

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
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