
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <MIDI.h>
#include "Settings.h"
#include "UI.h"

#define MIDI_RANGE 128
#define NOTE_DURATION 100

////Debug mmacros///

//Commet/uncomment next Line To toggle Serial Or MIDIç

//#define DEBUG

#ifdef DEBUG

#define PRINT(x) Serial.println(x)
#define SEND_ON(x, y, z)
#define SEND_OFF(x, y, z)

#else

#define PRINT(x)
#define SEND_ON(x, y, z) MIDI.sendNoteOn(x, y, z)
#define SEND_OFF(x, y, z) MIDI.sendNoteOff(x, y, z)

#endif

MIDI_CREATE_DEFAULT_INSTANCE();

int padNotes[] = {38, 36, 47};
int thresholds[] = {110, 110, 110};
int readForMicros[] = {3800, 3800, 3800};
int sensitivity[] = {1022, 1022, 1022}; // smaller is more sensity, should never go under PIEZO_THRESHOLD

unsigned long readingFor[PADNUM] = {0};
int maxRead[PADNUM] = {0};
int prevRead[PADNUM] = {0};
float slopes[PADNUM] = {0};

int minVel = 1; //min velocity sent

unsigned long debounce = 28;
unsigned long delays[PADNUM];

bool reading[PADNUM];
bool enabled[PADNUM];
bool noteOn[MIDI_RANGE];

int noteOnDuration[MIDI_RANGE] = {0};
int currentRead;

UI ui;

void setup()
{
  //MIDI_CHANNEL_OFF: disregard incoming messages (MIDI_CHANNEL_OMNI to listen to al channels)
  MIDI.begin(MIDI_CHANNEL_OFF);
  Serial.begin(115200); // set serial output rate
  startArrays();
  ui.oledSetup();
}

void loop()
{

  for (int i = 0; i < PADNUM; i++)
  {
    readAnalog(i);
  }

  checkTimers();
  ui.updateUI();
}

void readAnalog(int i)
{

  currentRead = analogRead(i);
  if (millis() < delays[i] + debounce)
  {
    return;
  }

  if (currentRead > thresholds[i])
  {
    PRINT(currentRead);
  }

  if (currentRead > thresholds[i] && enabled[i])
  {

    if (!reading[i])
    {
      reading[i] = true;
      readingFor[i] = micros();
    }
    if (maxRead[i] < currentRead)
    {
      maxRead[i] = currentRead;
    }

    prevRead[i] = currentRead;
  }

  if (enabled[i] && reading[i] && (micros() > readingFor[i] + readForMicros[i]))
  {

    filterAndSend(i);

    enabled[i] = false;
    maxRead[i] = 0;
    prevRead[i] = 0;
  }

  if (!enabled[i] && reading[i] && currentRead <= thresholds[i])
  {
    //Serial.println("PIEZO AT ZERO");
    delays[i] = millis();
    reading[i] = false;
    enabled[i] = true;
  }
}

void filterAndSend(int pad)
{

  //PRINT("AREA: ");
  //PRINT(maxRead[pad]);
  //float vel = ((maxRead[pad] - thresholds[pad]) / float(sensitivity[pad])) * 127;

  /* Note, "slope" below is a constant for given numbers, so if you are calculating
     a lot of output values, it makes sense to calculate it once.  It also makes
     understanding the code easier */

  int vel = 1 + slopes[pad] * (maxRead[pad] - thresholds[pad]);
  if (vel > 127)
    vel = 127;

  PRINT("SENT: ");
  PRINT(vel);

  sendData(pad, vel);
}

void sendData(int i, int vel)
{
  ui.cueLED(i, vel);
  cueNoteOff(padNotes[i]);
  SEND_ON(padNotes[i], vel, 1);
}

void checkTimers()
{

  //NOTES ON
  for (int i = 0; i < MIDI_RANGE; i++)
  {
    if (noteOn[i])
    {
      //Send note off if note has been on for NOTE_DURATION miliseconds
      if (millis() >= noteOnDuration[i] + NOTE_DURATION)
      {
        SEND_OFF(i, 0, 1);
        noteOn[i] = false;
      }
    }
  }
}

void cueNoteOff(int note)
{
  //Send note off if it was already playing befere cueing a note
  if (noteOn[note])
  {
    SEND_OFF(note, 0, 1);
  }
  noteOn[note] = true;
  noteOnDuration[note] = millis();
}

void startArrays()
{

  for (int i = 0; i < PADNUM; i++)
  {
    delays[i] = millis();
    reading[i] = false;
    enabled[i] = true;

    slopes[i] = 1.0 * (127 - 1) / (sensitivity[i] - thresholds[i]);
  }
}
