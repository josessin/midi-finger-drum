#include "Arduino.h"
#include "UI.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "Settings.h"
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
// Define proper RST_PIN if required.
#define RST_PIN -1

//CONSTRUCTOR
UI::UI()
{
    for (int i = 0; i < PADNUM; i++)
    {
        _ledTimers[i] = millis();
    }
}

void UI::updateUI()
{

    //LED
    for (int i = 0; i < PADNUM; i++)
    {
        if (millis() >= _ledTimers[i] + _ledDuration[i])
        {
            analogWrite(_leds[i], 0);
        }
    }
}

void UI::cueLED(int pad, int vel)
{
    analogWrite(_leds[pad], 10 + vel * 0.3);
    _ledDuration[pad] = 2 + vel * 0.75;
    _ledTimers[pad] = millis();
}

void UI::oledSetup()
{
    Wire.begin();
    Wire.setClock(400000L);

#if RST_PIN >= 0
    _oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else  // RST_PIN >= 0
    _oled.begin(&Adafruit128x32, I2C_ADDRESS);
#endif // RST_PIN >= 0

    _oled.setFont(Adafruit5x7);

    uint32_t m = micros();
    _oled.clear();
    // first row
    _oled.println("set1X jose");

    // second row
    _oled.set2X();
    _oled.println("set2X jose");

    // third row
    _oled.set1X();
    _oled.print("micros: ");
    _oled.print(micros() - m);
}
