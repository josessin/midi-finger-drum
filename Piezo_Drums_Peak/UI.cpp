
#include "UI.h"
#include "UI.h"
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

    for (int i = 0; i < PADNUM; i++)
    {
        pinMode(_selectedLeds[i], OUTPUT);
    }

    pinMode(ROT_A, INPUT);
    pinMode(ROT_B, INPUT);
    pinMode(ROT_SW, INPUT);
    //call updateEncoder() when any high/low changed seen
    //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
}

void UI::updateUI()
{

    //PWM leds timers
    for (int i = 0; i < PADNUM; i++)
    {
        if (millis() >= _ledTimers[i] + _ledDuration[i])
        {
            analogWrite(_pwmLeds[i], 0);
        }
    }
}

void UI::cueLED(int pad, int vel)
{
    analogWrite(_pwmLeds[pad], 10 + vel * 0.3);
    _ledDuration[pad] = 2 + vel * 0.75;
    _ledTimers[pad] = millis();
}

void UI::switchPressed(int i, int menu, int value)
{

    if (menu == 1)
    {
        _turnOtherLedsOff(i);
        digitalWrite(_selectedLeds[i], HIGH);
        if (menu != _selectedMenu || i != UI::currentSelectedIndex)
        {
            _oled.clear();
            _oled.set2X();
            // first row
            _oled.print("P");
            _oled.print(i + 1);
            _oled.println(" -NOTE- ");
        }
    }
    else if (menu == 2)
    {
        _turnOtherLedsOff(i);
        digitalWrite(_selectedLeds[i], HIGH);
        if (menu != _selectedMenu || i != UI::currentSelectedIndex)
        {
            _oled.clear();
            _oled.set2X();
            // first row
            _oled.print("P");
            _oled.print(i + 1);
            _oled.println(" -THRESH-");
        }
    }
    else if (menu == 3)
    {
        _turnOtherLedsOff(i);
        digitalWrite(_selectedLeds[i], HIGH);
        if (menu != _selectedMenu || i != UI::currentSelectedIndex)
        {
            _oled.clear();
            _oled.set2X();
            // first row
            _oled.print("P");
            _oled.print(i + 1);
            _oled.println(" -SENSIT-");
        }
    }
    else if (menu == 4)
    {
        _turnOtherLedsOff(i);
        digitalWrite(_selectedLeds[i], HIGH);
        if (menu != _selectedMenu || i != UI::currentSelectedIndex)
        {
            _oled.clear();
            _oled.set2X();
            // first row
            _oled.print("P");
            _oled.print(i + 1);
            _oled.println(" -READu-");
        }
    }
    else if (menu == 5)
    {
        _turnOtherLedsOff(i);
        digitalWrite(_selectedLeds[i], HIGH);
        if (menu != _selectedMenu || i != UI::currentSelectedIndex)
        {
            _oled.clear();
            _oled.set2X();
            // first row
            _oled.print("P");
            _oled.print(i + 1);
            _oled.println(" -DEBUN-");
        }
    }
    // second row
    _oled.clearToEOL();
    _oled.set2X();
    _oled.println(value);
    UI::currentSelectedIndex = i;
    _selectedMenu = menu;
}

void UI::turnOffLedsAndReturnHome()
{
    _selectedMenu = -1;
    currentSelectedIndex = -1;
    _turnOtherLedsOff(-1);
    _homeScreen();
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
    _homeScreen();
}

void UI::setOled(char *prop, int value)
{
    _oled.clear();
    _oled.set2X();
    // first row
    _oled.print(UI::currentSelectedIndex);
    _oled.print(" -> ");
    _oled.println(prop);
    // second row

    _oled.println(value);
}

void UI::_homeScreen()
{
    _oled.clear();
    _oled.set1X();
    // first row
    _oled.println("");

    // second row
    _oled.set2X();
    _oled.println("Hola Jose!");
}

void UI::_turnOtherLedsOff(int ledON)
{

    for (int i = 0; i < PADNUM; i++)
    {
        if (i != ledON)
        {
            digitalWrite(_selectedLeds[i], LOW);
            _selLedOn[i] = false;

            //turn off all leds but led [i]
        }
    }
}
