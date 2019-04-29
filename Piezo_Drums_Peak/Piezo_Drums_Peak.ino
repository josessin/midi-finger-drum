
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <MIDI.h>
#include "Settings.h"
#include "UI.h"
#include "Pad.h"
#include "enums.h"
#include "Switch.h"
#include "Encoder.h"

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

Pad pad[PADNUM];

int minVel = 1; //min velocity sent
bool noteOn[MIDI_RANGE];

int noteOnDuration[MIDI_RANGE] = {0};
int currentRead;

int currentSelectedPad = -1;
unsigned long selectePadTimer = 0;
uint8_t currentPadMenu = 0;

Switch switches[PADNUM];
Switch encoderSW;
Encoder encoder;

UI ui;

void setup()
{
  ///CHANGE IF MORE PADS ARE INCORPARATED
  pad[0].analogPin = ANALOG_0;
  pad[1].analogPin = ANALOG_1;
  pad[2].analogPin = ANALOG_2;

  pad[0].swPin = T_SW_0;
  pad[1].swPin = T_SW_1;
  pad[2].swPin = T_SW_2;
  //END CHANGE

  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ROT_A), encoderA, LOW);

  //MIDI_CHANNEL_OFF: disregard incoming messages (MIDI_CHANNEL_OMNI to listen to al channels)
  MIDI.begin(MIDI_CHANNEL_OFF);
  Serial.begin(115200); // set serial output rate
  startArrays();
  ui.oledSetup();

  for (int i = 0; i < PADNUM; i++)
  {
    pinMode(pad[i].swPin, INPUT_PULLUP);
  }
}

void loop()
{

  for (int i = 0; i < PADNUM; i++)
  {
    readAnalog(i);
  }

  checkTimers();
  checkInput();
  manageMenus();
  ui.updateUI();
}

void readAnalog(int i)
{

  currentRead = analogRead(i);
  if (millis() < pad[i].debounceTimer + pad[i].debounce)
  {
    return;
  }

  if (currentRead > pad[i].threshold && pad[i].enabled)
  {
    PRINT(currentRead);
    if (!pad[i].reading)
    {
      pad[i].reading = true;
      pad[i].readingFor = micros();
    }
    if (pad[i].maxRead < currentRead)
    {
      pad[i].maxRead = currentRead;
    }

    pad[i].prevRead = currentRead;
  }

  if (pad[i].enabled && pad[i].reading && (micros() > pad[i].readingFor + pad[i].readForMicros))
  {

    filterAndSend(i);

    pad[i].enabled = false;
    pad[i].maxRead = 0;
    pad[i].prevRead = 0;
  }

  if (!pad[i].enabled && pad[i].reading && currentRead <= pad[i].threshold)
  {
    PRINT("PIEZO AT ZERO");
    pad[i].debounceTimer = millis();
    pad[i].reading = false;
    pad[i].enabled = true;
  }
}

void filterAndSend(int hittedPad)
{

  PRINT("AREA: ");
  PRINT(pad[hittedPad].maxRead);

  int vel = 1 + pad[hittedPad].slope * (pad[hittedPad].maxRead - pad[hittedPad].threshold);
  if (vel > 127)
    vel = 127;

  PRINT("SENT: ");
  PRINT(vel);

  sendData(hittedPad, vel);
}

void sendData(int i, int vel)
{
  ui.cueLED(i, vel);
  cueNoteOff(pad[i].note);
  SEND_ON(pad[i].note, vel, 1);
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

  //PAD currentSelectedPad
  for (int i = 0; i < PADNUM; i++)
  {
    if (millis() >= selectePadTimer + SEL_LED_DURATION && currentSelectedPad != -1)
    {
      currentSelectedPad = -1;
      detachInterrupt(digitalPinToInterrupt(ROT_A));
      ui.turnOffLedsAndReturnHome();
    }
  }
}
void manageMenus()
{
  if (currentSelectedPad == -1)
  {
    return;
  }

  switch (currentPadMenu)
  {
  case 0:
    break;
  case 1:
    if (encoder.changed)
    {
      if (encoder.direction > 0)
      {
        pad[currentSelectedPad].note++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].note--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].note);
      //Reset timer (to allow more time)
      selectePadTimer = millis();
      resetEncoder();
    }
    break;
  case 2:
    if (encoder.changed)
    {

      if (encoder.direction > 0)
      {
        pad[currentSelectedPad].threshold++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].threshold--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].threshold);
      //Reset timer (to allow more time)
      selectePadTimer = millis();
      resetEncoder();
    }
    break;
  case 3:
    if (encoder.changed)
    {

      if (encoder.direction > 0)
      {
        pad[currentSelectedPad].sensitivity++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].sensitivity--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].sensitivity);
      //Reset timer (to allow more time)
      selectePadTimer = millis();
      resetEncoder();
    }
    break;
  case 4:
    if (encoder.changed)
    {

      if (encoder.direction > 0)
      {
        pad[currentSelectedPad].readForMicros += 10;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].readForMicros -= 10;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].readForMicros);
      //Reset timer (to allow more time)
      selectePadTimer = millis();
      resetEncoder();
    }
    break;
  case 5:
    if (encoder.changed)
    {

      if (encoder.direction > 0)
      {
        pad[currentSelectedPad].debounce++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].debounce--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].debounce);
      //Reset timer (to allow more time)
      selectePadTimer = millis();
      resetEncoder();
    }
    break;
  default:
    break;
  }
}
void resetEncoder()
{
  encoder.changed = false;
  encoder.position = 0;
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
    pad[i].debounceTimer = millis();
    pad[i].reading = false;
    pad[i].enabled = true;

    pad[i].slope = 1.0 * (127 - 1) / (pad[i].sensitivity - pad[i].threshold);
  }
}
void checkInput()
{
  //Pad Switches (with debounce)
  for (int i = 0; i < PADNUM; i++)
  {
    int swRead = digitalRead(pad[i].swPin);

    if (swRead != switches[i].lastState)
    {
      switches[i].debounceTimer = millis();
    }
    if ((millis() - switches[i].debounceTimer) > DEBOUNCE)
    {
      if (swRead != switches[i].currentState)
      {
        switches[i].currentState = swRead;
        attachInterrupt(digitalPinToInterrupt(ROT_A), encoderA, LOW);
        if (switches[i].currentState == LOW && i != currentSelectedPad)
        {
          selectePadTimer = millis();
          currentSelectedPad = i;
          currentPadMenu = 1;
          ui.switchPressed(i, currentPadMenu, pad[i].note);
        }
        else if (switches[i].currentState == LOW && i == currentSelectedPad)
        {
          selectePadTimer = millis();
          currentSelectedPad = i;
          currentPadMenu++;
          if (currentPadMenu == 2)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].threshold);
          }
          else if (currentPadMenu == 3)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].sensitivity);
          }
          else if (currentPadMenu == 4)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].readForMicros);
          }
          else if (currentPadMenu == 5)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].debounce);
          }else{
            currentPadMenu = 1;
            ui.switchPressed(i, currentPadMenu, pad[i].note);
          }
        }
      }
    }
    switches[i].lastState = swRead;
  }
}

void encoderA()
{
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 5)
  {
    if (digitalRead(ROT_B) == LOW)
    {
      encoder.position--; // Could be -5 or -10
      encoder.direction = -1;
    }
    else
    {
      encoder.position++; // Could be -5 or -10
      encoder.direction = 1;
    }
    encoder.changed = true;
    // Restrict value from 0 to +100
    //virtualPosition = min(100, max(0, virtualPosition));
  }
  // Keep track of when we were here last (no more than every 5ms)
  lastInterruptTime = interruptTime;
}
