
#ifndef UI_h
#define UI_h
#include "Arduino.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "Settings.h"

class UI
{
    SSD1306AsciiWire _oled;
    unsigned long _ledTimers[PADNUM];
    int _ledDuration[PADNUM] = {0};
    int _leds[3] = {10, 11, 9};

  public:
    UI();
    void updateUI();
    void cueLED(int pad, int vel);
    void oledSetup();
};

#endif
