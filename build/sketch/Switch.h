#ifndef Switch_h
#define Switch_h
#include "Arduino.h"
#include "Settings.h"

typedef struct {

    unsigned long int debounceTimer = 0;
    int lastState = 0;
    int currentState = 0;

}Switch;

#endif