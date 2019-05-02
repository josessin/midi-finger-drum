# 1 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
# 1 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"

# 3 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 4 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 5 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 6 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 7 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 8 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 9 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 10 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 11 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 12 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2
# 13 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino" 2

////Debug mmacros///

//Commet/uncomment next Line To toggle Serial Or MIDIç

//#define DEBUG
# 34 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial);;;

Pad pad[3];

int minVel = 1; //min velocity sent
bool noteOn[128];

unsigned long noteOnDuration[128];
int currentRead;

int currentSelectedPad = -1;
unsigned long selectePadTimer = 0;
uint8_t currentPadMenu = 0;
uint8_t prog = 0;
Switch switches[3];
Switch encoderSW;
Encoder encoder;

UI ui;

void setup()
{
  ///CHANGE IF MORE PADS ARE INCORPARATED
  pad[0].analogPin = 0;
  pad[1].analogPin = 1;
  pad[2].analogPin = 2;

  pad[0].swPin = 5;
  pad[1].swPin = 17 /*A3*/;
  pad[2].swPin = 4;
  //END CHANGE

  pinMode(3, 0x0);
  pinMode(2, 0x0);

  attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)), encoderA, 0x0);

  //MIDI_CHANNEL_OFF: disregard incoming messages (MIDI_CHANNEL_OMNI to listen to al channels)
  MIDI.begin(17 /* and over*/);
  Serial.begin(115200); // set serial output rate
  startArrays();
  ui.oledSetup();

  for (int i = 0; i < 3; i++)
  {
    pinMode(pad[i].swPin, 0x2);
  }
# 91 "d:\\Project MIDI\\Piezo_Drums_Peak\\Piezo_Drums_Peak.ino"
  loadPads();
}

void loop()
{

  for (int i = 0; i < 3; i++)
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
    ;
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
    ;
    pad[i].debounceTimer = millis();
    pad[i].reading = false;
    pad[i].enabled = true;
  }
}

void filterAndSend(int hittedPad)
{

  ;
  ;

  int vel = 1 + pad[hittedPad].slope * (pad[hittedPad].maxRead - pad[hittedPad].settings[prog].threshold);
  if (vel > 127)
    vel = 127;

  ;
  ;

  sendData(hittedPad, vel);
}

void sendData(int i, int vel)
{
  cueNoteOff(pad[i].settings[prog].note);
  Serial.flush();
  MIDI.sendNoteOn(pad[i].settings[prog].note, vel, 1);
  ui.cueLED(i, vel);
}

void cueNoteOff(int note)
{

  //Send note off if it was already playing befere cueing a note
  if (noteOn[note])
  {
    MIDI.sendNoteOff(note, 0, 1);
  }
  noteOnDuration[note] = millis();
  noteOn[note] = true;
}

void checkTimers()
{

  //NOTES ON
  for (int i = 0; i < 128; i++)
  {
    if (noteOn[i])
    {
      //Send note off if note has been on for NOTE_DURATION miliseconds
      if (millis() >= noteOnDuration[i] + 120)
      {
        noteOn[i] = false;
        MIDI.sendNoteOff(i, 0, 1);
      }
    }
  }

  //PAD currentSelectedPad
  for (int i = 0; i < 3; i++)
  {
    if (millis() >= selectePadTimer + 5000 && currentSelectedPad != -1)
    {
      currentSelectedPad = -1;
      detachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)));
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

  for (int i = 0; i < 3; i++)
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
  for (int i = 0; i < 3; i++)
  {
    int swRead = digitalRead(pad[i].swPin);

    if (swRead != switches[i].lastState)
    {
      switches[i].debounceTimer = millis();
    }
    if ((millis() - switches[i].debounceTimer) > 50)
    {
      if (swRead != switches[i].currentState)
      {
        switches[i].currentState = swRead;

        if (switches[i].currentState == 0x0 && i != currentSelectedPad)
        {
          attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)), encoderA, 0x0);
          selectePadTimer = millis();
          currentSelectedPad = i;
          currentPadMenu = 1;
          ui.switchPressed(i, currentPadMenu, pad[i].settings[prog].note, prog);
        }
        else if (switches[i].currentState == 0x0 && i == currentSelectedPad)
        {
          attachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)), encoderA, 0x0);
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

  int rotSWRead = digitalRead(13);

  if (rotSWRead != encoderSW.lastState)
  {
    encoderSW.debounceTimer = millis();
  }
  if ((millis() - encoderSW.debounceTimer) > 50)
  {
    if (rotSWRead != encoderSW.currentState)
    {
      encoderSW.currentState = rotSWRead;
      if (encoderSW.currentState == 0x1)
      {

        prog++;
        if (prog >= 2)
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
  int address = sizeof(PadSettings) * (i * 2 + prog);
  //Serial.println(address);
  EEPROM.put(address, pad[i].settings[prog]);
}

void loadPads()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      int address = sizeof(PadSettings) * (i * 2 + j);
      EEPROM.get(address, pad[i].settings[j]);
    }
  }
}

void clearPads()
{
  //PAD currentSelectedPad
  for (int i = 0; i < 3; i++)
  {
    currentSelectedPad = -1;
    detachInterrupt(((3) == 2 ? 0 : ((3) == 3 ? 1 : -1)));
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
    if (digitalRead(2) == 0x0)
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
