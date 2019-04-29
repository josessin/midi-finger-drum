#ifndef Encoder_h
#define Encoder_h
#include "Arduino.h"
#include "Settings.h"

typedef struct
{
    volatile int direction = 0; //-1,0,1
    volatile int position = 0;
    volatile bool changed = false;

} Encoder;

#endif