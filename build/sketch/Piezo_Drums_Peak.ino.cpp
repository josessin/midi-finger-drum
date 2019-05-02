#include <Arduino.h>
#line 1 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
#line 1 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"

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
#include <EEPROM.h>

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

unsigned long noteOnDuration[MIDI_RANGE];
int currentRead;

int currentSelectedPad = -1;
unsigned long selectePadTimer = 0;
uint8_t currentPadMenu = 0;
uint8_t prog = 0;
Switch switches[PADNUM];
Switch encoderSW;
Encoder encoder;

UI ui;

#line 54 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void setup();
#line 94 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void loop();
#line 108 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void readAnalog(int i);
#line 152 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void filterAndSend(int hittedPad);
#line 168 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void sendData(int i, int vel);
#line 176 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void cueNoteOff(int note);
#line 188 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void checkTimers();
#line 217 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void manageMenus();
#line 332 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void resetEncoder();
#line 338 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void startArrays();
#line 350 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void calcSlope(int i);
#line 355 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void checkInput();
#line 440 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void savePad(int i, int prog);
#line 447 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void loadPads();
#line 459 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void clearPads();
#line 470 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
void encoderA();
#line 54 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
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

#ifdef RESTORE_SETTINGS
  for (int i = 0; i < PADNUM; i++)
  {
    for (int j = 0; j < PROGNUM; j++)
    {
      savePad(i, j);
    }
  }
#endif
  loadPads();
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
  if (millis() < pad[i].debounceTimer + pad[i].settings[prog].debounce)
  {
    return;
  }

  if (currentRead > pad[i].settings[prog].threshold && pad[i].enabled)
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

  if (pad[i].enabled && pad[i].reading && (micros() > pad[i].readingFor + pad[i].settings[prog].readForMicros))
  {

    filterAndSend(i);

    pad[i].enabled = false;
    pad[i].maxRead = 0;
    pad[i].prevRead = 0;
  }

  if (!pad[i].enabled && pad[i].reading && currentRead <= pad[i].settings[prog].threshold)
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

  int vel = 1 + pad[hittedPad].slope * (pad[hittedPad].maxRead - pad[hittedPad].settings[prog].threshold);
  if (vel > 127)
    vel = 127;

  PRINT("SENT: ");
  PRINT(vel);

  sendData(hittedPad, vel);
}

void sendData(int i, int vel)
{
  cueNoteOff(pad[i].settings[prog].note);
  Serial.flush();
  SEND_ON(pad[i].settings[prog].note, vel, 1);
  ui.cueLED(i, vel);
}

void cueNoteOff(int note)
{

  //Send note off if it was already playing befere cueing a note
  if (noteOn[note])
  {
    SEND_OFF(note, 0, 1);
  }
  noteOnDuration[note] = millis();
  noteOn[note] = true;
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
        noteOn[i] = false;
        SEND_OFF(i, 0, 1);
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
        pad[currentSelectedPad].settings[prog].note++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].settings[prog].note--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].settings[prog].note, prog);
      savePad(currentSelectedPad, prog);
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
        pad[currentSelectedPad].settings[prog].threshold++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].settings[prog].threshold--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].settings[prog].threshold, prog);
      savePad(currentSelectedPad, prog);
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
        pad[currentSelectedPad].settings[prog].sensitivity += 10;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].settings[prog].sensitivity -= 10;
      }
      calcSlope(currentSelectedPad);
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].settings[prog].sensitivity, prog);
      savePad(currentSelectedPad, prog);
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
        pad[currentSelectedPad].settings[prog].readForMicros += 50;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].settings[prog].readForMicros -= 50;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].settings[prog].readForMicros, prog);
      savePad(currentSelectedPad, prog);
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
        pad[currentSelectedPad].settings[prog].debounce++;
      }
      else if (encoder.direction < 0)
      {
        pad[currentSelectedPad].settings[prog].debounce--;
      }
      //Update display
      ui.switchPressed(currentSelectedPad, currentPadMenu, pad[currentSelectedPad].settings[prog].debounce, prog);
      savePad(currentSelectedPad, prog);
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

void startArrays()
{

  for (int i = 0; i < PADNUM; i++)
  {
    pad[i].debounceTimer = millis();
    pad[i].reading = false;
    pad[i].enabled = true;
    calcSlope(i);
  }
}

void calcSlope(int i)
{
  pad[i].slope = 1.0 * (127 - 1) / (pad[i].settings[prog].sensitivity - pad[i].settings[prog].threshold);
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

        if (switches[i].currentState == LOW && i != currentSelectedPad)
        {
          attachInterrupt(digitalPinToInterrupt(ROT_A), encoderA, LOW);
          selectePadTimer = millis();
          currentSelectedPad = i;
          currentPadMenu = 1;
          ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].note, prog);
        }
        else if (switches[i].currentState == LOW && i == currentSelectedPad)
        {
          attachInterrupt(digitalPinToInterrupt(ROT_A), encoderA, LOW);
          selectePadTimer = millis();
          currentSelectedPad = i;
          currentPadMenu++;
          if (currentPadMenu == 2)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].threshold, prog);
          }
          else if (currentPadMenu == 3)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].sensitivity, prog);
          }
          else if (currentPadMenu == 4)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].readForMicros, prog);
          }
          else if (currentPadMenu == 5)
          {
            ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].debounce, prog);
          }
          else
          {
            currentPadMenu = 1;
            ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].note, prog);
          }
        }
      }
    }
    switches[i].lastState = swRead;
  }

  int rotSWRead = digitalRead(ROT_SW);

  if (rotSWRead != encoderSW.lastState)
  {
    encoderSW.debounceTimer = millis();
  }
  if ((millis() - encoderSW.debounceTimer) > DEBOUNCE)
  {
    if (rotSWRead != encoderSW.currentState)
    {
      encoderSW.currentState = rotSWRead;
      if (encoderSW.currentState == HIGH)
      {

        prog++;
        if (prog >= PROGNUM)
        {
          prog = 0;
        }
        clearPads();
        ui.setOled("Program->", prog + 1);
      }
    }
  }
  encoderSW.lastState = rotSWRead;
}

void savePad(int i, int prog)
{
  int address = sizeof(PadSettings) * (i * PROGNUM + prog);
  //Serial.println(address);
  EEPROM.put(address, pad[i].settings[prog]);
}

void loadPads()
{
  for (int i = 0; i < PADNUM; i++)
  {
    for (int j = 0; j < PROGNUM; j++)
    {
      int address = sizeof(PadSettings) * (i * PROGNUM + j);
      EEPROM.get(address, pad[i].settings[j]);
    }
  }
}

void clearPads()
{
  //PAD currentSelectedPad
  for (int i = 0; i < PADNUM; i++)
  {
    currentSelectedPad = -1;
    detachInterrupt(digitalPinToInterrupt(ROT_A));
  }
  ui.turnLedsOff();
}

void encoderA()
{
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 3)
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

