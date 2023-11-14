#include "Arduino.h"
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"


//-------------------------------------------
// folgende Konstanten können angepasst werden. Die Zeiten sind in millisekunden.

//sensor1 -> handsensor
//sensor2 -> präsenzsensor
const int sensor1_pin = A0; // connect IR sensor module to Arduino pin A0
const int sensor2_pin = A1; // TODO: connect präsenz sensor module to Arduino pin XX, choose and ajust pin
const int relay_pin = 2;

const int sound1_playFor = 5000; //how long should sound 1 be played?
const int sound2_playFor = 5000; //how long should sound 2 be played?
const int relay_switchFor = 1000; // how long should the relay be switched?

const int sound1_backoff = 7000; // how long should the sound 1 not be played after it has been played
const int sound2_backoff = 30000; // how long should the sound 2 not be played after it has been played
const int relay_backoff = 3000; // how long should the relay not be switched after it has been through a switch cycle


// --------------------------------------------------
//internal variables

unsigned long sound1_lastPlayStart = 0;
unsigned long sound1_lastPlayStop = 1;

unsigned long sound2_lastPlayStart = 0;
unsigned long sound2_lastPlayStop = 1;


unsigned long relay_lastTriggerOn = 0;
unsigned long relay_lastTriggerOff = 1;

static unsigned long currentTime;

SoftwareSerial DFPlayerSoftwareSerial(10,11);// RX, TX
DFRobotDFPlayerMini mp3Player;

bool checkTimeOver(unsigned long lastTrigger, unsigned long triggerDelay);
bool isCurrentlyInAction(unsigned long actionStart, unsigned long actionEnd);
bool isCurrentlyNotInAction(unsigned long actionStart, unsigned long actionEnd);
void handleRelaySwitchOn();
void handleRelaySwitchOff();
void handleSound1Start();
void handleSound1Stop();
void handleSound2Start();
void handleSound2Stop();

void setup() {
  pinMode(sensor1_pin, INPUT);
  pinMode(sensor2_pin, INPUT);
  pinMode(relay_pin, OUTPUT);

  //Serial.begin(9600);
  DFPlayerSoftwareSerial.begin(9600);
  Serial.begin(9600); // DFPlayer Mini mit SoftwareSerial initialisieren
  mp3Player.begin(DFPlayerSoftwareSerial, false, true);
  mp3Player.volume(30); // Lautstärke auf 20 (0-30) setzen
}

void loop() {
  // Sensor-Zustand überprüfen
  bool sensor1_isOn = digitalRead(sensor1_pin);
  bool sensor2_isOn = digitalRead(sensor2_pin);

  currentTime = millis();

  if(sensor1_isOn) {
    handleRelaySwitchOn();
    handleSound1Start();
  }

  if(sensor2_isOn) {
    handleSound2Start();
  }

  handleRelaySwitchOff();
  handleSound1Stop();
  handleSound2Stop();
}

// Die folgenden Methoden funktionieren alle gleich:
// Es wird geprüft, ob man gerade eine aktion ausführt.
// Eine Aktion kann nur ausgeführt werden, wenn sie gerade nicht ausgeführt wird
// Dann wird geschaut, ob eine gewisse Zeit um ist, sodass eine Aktion nicht zu schnell hintereinander ausgeführt werden kann
// sollte beides gegebnen sein, dann wird die Aktion ausgeführt (switch relay/play/pause)

// das gleich Prinzip gilt auch für Aktionen, die gerade ausgeführt werden und ausgeschaltet werden sollen.

void handleRelaySwitchOn() {
  if(isCurrentlyNotInAction(relay_lastTriggerOff, relay_lastTriggerOn)) {
    //relay can be switched on
    if(checkTimeOver(relay_lastTriggerOff, relay_backoff)) {
      //switch soap on
      digitalWrite(relay_pin, LOW); 
      relay_lastTriggerOn = currentTime;
    }
  }
}

void handleRelaySwitchOff() {
  if(isCurrentlyInAction(relay_lastTriggerOff, relay_lastTriggerOn)) {
    //relay can be switched off
    if(checkTimeOver(relay_lastTriggerOn, relay_switchFor)) {
      //switch soap off
      digitalWrite(relay_pin, HIGH); 
      relay_lastTriggerOff = currentTime;
    }
  }
}

void handleSound1Start(){
  if(isCurrentlyNotInAction(sound1_lastPlayStart, sound1_lastPlayStop)) {
    //sound can be started
    if(checkTimeOver(sound1_lastPlayStop, sound1_backoff)) {
      //start sound1
      mp3Player.play(1);
      sound1_lastPlayStart = currentTime;

      //pretent like sound 2 has just been stopped, so it wont play for the backoff time amount of sound 2
      sound2_lastPlayStop = currentTime;
    }
  }
}

void handleSound1Stop() {
  if(isCurrentlyInAction(sound1_lastPlayStart, sound1_lastPlayStop)) {
    //sound can be stopped
    if(checkTimeOver(sound1_lastPlayStart, sound1_playFor)) {
      //stop sound1
      mp3Player.stop();
      sound1_lastPlayStop = currentTime;
    }
  }
}

void handleSound2Start() {
  if(isCurrentlyNotInAction(sound2_lastPlayStart, sound2_lastPlayStop))  {
    //sound can be started
    if(checkTimeOver(sound2_lastPlayStop, sound2_backoff)) {
      //start sound1
      mp3Player.play(2);
      sound2_lastPlayStart = currentTime;
    }
  }
}

void handleSound2Stop() {
  if(isCurrentlyInAction(sound2_lastPlayStart, sound2_lastPlayStop)) {
    //sound can be stopped
    if(checkTimeOver(sound2_lastPlayStart, sound2_playFor)) {
      //stop sound1
      mp3Player.stop();
      sound2_lastPlayStop = currentTime;
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