
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"


//-------------------------------------------
// folgende Konstanten können angepasst werden. Die Zeiten sind in millisekunden.

//sensor1 -> handsensor
//sensor2 -> präsenzsensor
//sensor2 -> shake sensor
const int sensor1_pin = A0; // connect IR hand sensor module to Arduino pin A0
const int sensor2_pin = A1; // praesense sensor module to Arduino pin A1
const int sensor3_pin = A2; // sensor module for tamper detection to Arduino pin A2
const int relay1_pin = 2; // relay 1 on pin D2 soap dispenser
const int relay2_pin = 3; // relay 2 on pin D3 kookoo bird
const int LED1_pin = A3; // LED

const int DFPlayer_arduinoRX = 10; // pin D10 -> RX on arduino TX on DFPlayer
const int DFPlayer_arduinoTX = 11; // pin D11 -> TX on arduino RX on DFPlayer

const unsigned  int sound1_playFor = 5000; //how long should sound 1 be played?
const unsigned  int sound2_playFor = 3000; //how long should sound 2 be played?
const unsigned  int sound3_playFor = 2000; //how long should sound 3 be played?
const unsigned  int relay1_switchFor = 550; // how long should the relay1 be switched?
const unsigned  int relay2_switchFor = 500; // how long should the relay2 be switched?

const unsigned  int sound1_backoff = 10000 + relay2_switchFor; // how long should the sound 1 not be played after it has been played
const unsigned  int sound2_backoff = 60000; // how long should the sound 2 not be played after it has been played
const unsigned  int sound3_backoff = 15000; // how long should the sound 3 not be played after it has been played
const unsigned  int relay1_backoff = 2000; // how long should the relay not be switched after it has been through a switch cycle
const unsigned  int relay2_backoff = 10000 + sound1_playFor; // how long should the relay not be switched after it has been through a switch cycle

const unsigned  int ledBlinkInterval = 1000;

const unsigned  int volume = 30; // sound volume 0-30 

const unsigned  int shakeSensitivity = 20; // up to 1024. The higher the number, the lower the sensitivity



// --------------------------------------------------
//internal variables

unsigned long sound1_lastPlayStart = 0;
unsigned long sound1_lastPlayStop = 1;

unsigned long sound2_lastPlayStart = 0;
unsigned long sound2_lastPlayStop = 1;

unsigned long sound3_lastPlayStart = 0;
unsigned long sound3_lastPlayStop = 1;

unsigned long relay1_lastTriggerOn = 0;
unsigned long relay1_lastTriggerOff = 1;

unsigned long relay2_lastTriggerOn = 0;
unsigned long relay2_lastTriggerOff = 1;

unsigned long led1_lastTriggerOn = 0;
unsigned long led1_lastTriggerOff = 1;

bool sensor1_lastState = false;
bool sensor2_lastState = false;
bool sensor3_lastState = false;

static unsigned long currentTime;
static unsigned long rememberShakeTime;
bool shakeNeedsToBeHandled = false;

SoftwareSerial DFPlayerSoftwareSerial(10,11);// RX, TX
DFRobotDFPlayerMini mp3Player;

//announce methodes we gonna use
bool checkTimeOver(unsigned long lastTrigger, unsigned long triggerDelay);
bool isCurrentlyInAction(unsigned long actionStart, unsigned long actionEnd);
bool isCurrentlyNotInAction(unsigned long actionStart, unsigned long actionEnd);
void switchSensor1RelatedStuffOn();
void switchSensor2RelatedStuffOn();
void switchSensor3RelatedStuffOn();
void handleRelay1SwitchOn();
void handleRelay1SwitchOff();
void handleSound1Start();
void handleSound1Stop();
void handleSound2Start();
void handleSound2Stop();
void handleSound3Start();
void handleSound3Stop();
void ledSwitch();

void setup() {
  pinMode(sensor1_pin, INPUT);
  pinMode(sensor2_pin, INPUT);
  pinMode(sensor3_pin, INPUT);
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_pin, OUTPUT);

  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);

  //Serial.begin(9600);
  DFPlayerSoftwareSerial.begin(9600);
  Serial.begin(9600); // DFPlayer Mini mit SoftwareSerial initialisieren
  mp3Player.begin(DFPlayerSoftwareSerial, false, true);
  mp3Player.volume(volume); // Lautstärke auf 20 (0-30) setzen

  delay(500);
}


void loop() {
  delay(20);
  // Sensor-Zustand überprüfen
  bool sensor1_isOn = !digitalRead(sensor1_pin);
  bool sensor2_isOn = digitalRead(sensor2_pin);
  int sensor3_value = analogRead(sensor3_pin);

  //display sensor1 state with buldin led
  digitalWrite(LED_BUILTIN, sensor1_isOn);

  ledSwitch();

  currentTime = millis() + 100000; //give it a little headstart / offset

  if(sensor1_isOn && sensor1_lastState == false) {
    Serial.print("Sensor_1 is on ");
    Serial.println(currentTime);
    switchSensor1RelatedStuffOn();
    sensor1_lastState = true;
  } else if(!sensor1_isOn && sensor1_lastState == true) {
    sensor1_lastState = false;
  }

  if(sensor2_isOn && sensor2_lastState == false) {
    Serial.print("Sensor_2 is on ");
    Serial.println(currentTime);
    switchSensor2RelatedStuffOn();
    sensor2_lastState = true;
  } else if(!sensor2_isOn && sensor2_lastState == true) {
    sensor2_lastState = false;
  }

  //shake sensor:
  // detect shake
  // see if x time before or wait for x time after for soap dispenser
  // if no soap dispenser active, then start sound

  if(sensor3_value > shakeSensitivity && sensor3_lastState == false && !shakeNeedsToBeHandled) {
    Serial.print("Sensor_3 shake detected. Shake amount: ");
    Serial.print(sensor3_value);
    Serial.print(" time: ");
    Serial.println(currentTime);
    rememberShakeTime = currentTime;
    shakeNeedsToBeHandled = true;
    sensor3_lastState = true;
  } else if(sensor3_value <= shakeSensitivity && sensor3_lastState == true) {
    sensor3_lastState = false;
  }

  
  //TODO: shorten time

  if(shakeNeedsToBeHandled) {
    if( currentTime - relay1_lastTriggerOn < 10 * 1000) {
      //there was a soap dispense event less than 30 secends ago and shake doesnt need to be handeled
      shakeNeedsToBeHandled = false;
    } else if (currentTime - rememberShakeTime > 5 * 1000) {
      //if the shaketime is 5 seconds ago with no soap dispenser event happening
      //then sound alarm
      switchSensor3RelatedStuffOn();
      shakeNeedsToBeHandled = false;
    }
  }

  handleRelay1SwitchOff();
  handleRelay2SwitchOff();
  handleSound1Stop();
  handleSound2Stop();
  handleSound3Stop();
}

void switchSensor1RelatedStuffOn() {
  //pretent like sound 2 has just been stopped, so it wont play for the backoff time amount of sound 2
  sound2_lastPlayStop = currentTime;
  
  handleRelay1SwitchOn();
  handleRelay2SwitchOn();
  //handleSound1Start();
}

void switchSensor2RelatedStuffOn() {
  handleSound2Start();
}

void switchSensor3RelatedStuffOn() {
  handleSound3Start();
}

// Die folgenden Methoden funktionieren alle gleich:
// Es wird geprüft, ob man gerade eine aktion ausführt.
// Eine Aktion kann nur ausgeführt werden, wenn sie gerade nicht ausgeführt wird
// Dann wird geschaut, ob eine gewisse Zeit um ist, sodass eine Aktion nicht zu schnell hintereinander ausgeführt werden kann
// sollte beides gegebnen sein, dann wird die Aktion ausgeführt (switch relay/play/pause)

// das gleich Prinzip gilt auch für Aktionen, die gerade ausgeführt werden und ausgeschaltet werden sollen.

void handleRelay1SwitchOn() {
  if(isCurrentlyNotInAction(relay1_lastTriggerOn, relay1_lastTriggerOff)) {
    //relay can be switched on
    if(checkTimeOver(relay1_lastTriggerOff, relay1_backoff)) {
      Serial.println("switch soap on");
      digitalWrite(relay1_pin, LOW); 
      relay1_lastTriggerOn = currentTime;
    }
  }
}

void handleRelay1SwitchOff() {
  if(isCurrentlyInAction(relay1_lastTriggerOn, relay1_lastTriggerOff)) {
    //relay can be switched off
    if(checkTimeOver(relay1_lastTriggerOn, relay1_switchFor)) {
      Serial.println("switch soap off");
      digitalWrite(relay1_pin, HIGH); 
      relay1_lastTriggerOff = currentTime;
    }
  }
}

void handleRelay2SwitchOn() {
  if(isCurrentlyNotInAction(relay2_lastTriggerOn, relay2_lastTriggerOff)) {
    //relay can be switched on
    if(checkTimeOver(relay2_lastTriggerOff, relay2_backoff)) {
      Serial.println("switch bird on");
      digitalWrite(relay2_pin, LOW); 
      relay2_lastTriggerOn = currentTime;
    }
  }
}

void handleRelay2SwitchOff() {
  if(isCurrentlyInAction(relay2_lastTriggerOn, relay2_lastTriggerOff)) {
    //relay can be switched off
    if(checkTimeOver(relay2_lastTriggerOn, relay2_switchFor)) {
      Serial.println("switch bird off");
      digitalWrite(relay2_pin, HIGH); 
      relay2_lastTriggerOff = currentTime;
    }
  }
}

void handleSound1Start(){
  if(isCurrentlyNotInAction(sound1_lastPlayStart, sound1_lastPlayStop)) {
    //sound can be started
    if(checkTimeOver(sound1_lastPlayStop, sound1_backoff)) {
      //start sound1
      Serial.println("start sound1");
      mp3Player.play(1);
      sound1_lastPlayStart = currentTime;
    }
  }
}

void handleSound1Stop() {
  if(isCurrentlyInAction(sound1_lastPlayStart, sound1_lastPlayStop)) {
    //sound can be stopped
    if(checkTimeOver(sound1_lastPlayStart, sound1_playFor)) {
      Serial.println("stop sound1");
      mp3Player.stop();
      sound1_lastPlayStop = currentTime;
    }
  }
}

void handleSound2Start() {
  if(isCurrentlyNotInAction(sound2_lastPlayStart, sound2_lastPlayStop))  {
    
    //sound can be started
    if(checkTimeOver(sound2_lastPlayStop, sound2_backoff)) {
      Serial.println("start sound2");
      mp3Player.play(2);
      sound2_lastPlayStart = currentTime;
    }
  }
}

void handleSound2Stop() {
  if(isCurrentlyInAction(sound2_lastPlayStart, sound2_lastPlayStop)) {
    //sound can be stopped
    if(checkTimeOver(sound2_lastPlayStart, sound2_playFor)) {
      Serial.println("stop sound2");
      mp3Player.stop();
      sound2_lastPlayStop = currentTime;
    }
  }
}

void handleSound3Start() {
  if(isCurrentlyNotInAction(sound3_lastPlayStart, sound3_lastPlayStop))  {
    //sound can be started
    if(checkTimeOver(sound3_lastPlayStop, sound3_backoff)) {
      Serial.println("start sound3");
      mp3Player.play(3);
      sound3_lastPlayStart = currentTime;
    }
  }
}

void handleSound3Stop() {
  if(isCurrentlyInAction(sound3_lastPlayStart, sound3_lastPlayStop)) {
    //sound can be stopped
    if(checkTimeOver(sound3_lastPlayStart, sound3_playFor)) {
      Serial.println("stop sound3");
      mp3Player.stop();
      sound3_lastPlayStop = currentTime;
    }
  }
}


void ledSwitch() {
  if(isCurrentlyInAction(led1_lastTriggerOn,led1_lastTriggerOff)) {
    if(checkTimeOver(led1_lastTriggerOn, ledBlinkInterval)) {
      //Serial.println("switch led off");
      digitalWrite(LED1_pin, LOW); 
      led1_lastTriggerOff = currentTime;
    }
  } else {
    if(checkTimeOver(led1_lastTriggerOff, ledBlinkInterval)) {
      //Serial.println("switch led on");
      digitalWrite(LED1_pin, HIGH); 
      led1_lastTriggerOn = currentTime;
    }
  }
}

bool checkTimeOver(unsigned long lastTrigger, unsigned long triggerDelay) {
  return lastTrigger + triggerDelay < currentTime;
}

bool isCurrentlyInAction(unsigned long actionStart, unsigned long actionEnd) {
  return actionStart >= actionEnd;
}

bool isCurrentlyNotInAction(unsigned long actionStart, unsigned long actionEnd) {
  return ! isCurrentlyInAction(actionStart, actionEnd);
}

