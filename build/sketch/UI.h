
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
  int _pwmLeds[PADNUM] = {PWM_LED_0, PWM_LED_1, PWM_LED_2};
  bool _selLedOn[PADNUM] = {false, false, false};
  unsigned int _selLedTimer = 0;
  int _selectedLeds[PADNUM] = {SEL_LED_0, SEL_LED_1, SEL_LED_2};
  int _selectedMenu = 0;
  int _lastValue = 0;

  const char *note_substring[12] =
      {
          "C", "C#", "D", "D#",
          "E", "F", "F#", "G",
          "G#", "A", "A#", "B"};

public:
  UI();
  void updateUI();
  void cueLED(int pad, int vel);
  void oledSetup();
  void setOled(char *n, int v);
  void switchPressed(int i, int menu, int value, int prog);
  void turnOffLedsAndReturnHome();
  void turnLedsOff();
  int currentSelectedIndex = -1;

private:
  void _checkInput();
  void _turnOtherLedsOff(int i);

  void _homeScreen();
};

#endif
