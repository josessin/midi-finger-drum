
#ifndef PAd_h
#define PAd_h
#include "Arduino.h"
#include "Settings.h"

typedef struct 
{
    int note = NOTE;
    int analogPin = 0;
    int sensitivity = SENSITIVITY;
    int threshold = THRESH;
    int readForMicros = READ_FOR_MICROS;
    int debounce = ANALOG_DEBOUNCE;
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