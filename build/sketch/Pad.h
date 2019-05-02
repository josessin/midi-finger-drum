
#ifndef PAd_h
#define PAd_h
#include "Arduino.h"
#include "Settings.h"

typedef struct 
{
    int note = NOTE;
    int sensitivity = SENSITIVITY;
    int threshold = THRESH;
    int readForMicros = READ_FOR_MICROS;
    int debounce = ANALOG_DEBOUNCE;
}PadSettings;

typedef struct 
{
    int analogPin = 0;
    PadSettings settings[PROGNUM];
    unsigned long readingFor = 0;
    int maxRead = 0;
    int prevRead = 0;
    float slope = 0;
    unsigned long debounceLenght = 0;
    unsigned long debounceTimer = 0;
    bool reading = false;
    bool enabled = true;
    int swPin = 0;
}Pad;

#endif