
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "JobManager.cpp"


//-------------------------------------------
// folgende Konstanten können angepasst werden. Die Zeiten sind in millisekunden.

//sensor1 -> handsensor
//sensor2 -> präsenzsensor
//sensor2 -> shake sensor
const uint8_t hand_pin = A0; // connect IR hand sensor module to Arduino pin A0
const uint8_t room_pin = A1; // praesense sensor module to Arduino pin A1
const uint8_t shake_pin = A2; // sensor module for tamper detection to Arduino pin A2

const uint8_t LED1_pin = 9; //D9 LED

const uint8_t pump_pin = 7; //D7 soap pump

const uint8_t birdFlap_pin = 6; //D6 bird flap 

const uint8_t birdMotor1Gnd_pin = 5; //D5 
const uint8_t birdMotor1Vcc_pin = 2; //D2

const uint8_t birdMotor2Gnd_pin = 3; //D3 
const uint8_t birdMotor2Vcc_pin = 4; //D4 

const uint8_t DFPlayer_arduinoRX_pin = 10; //D10 -> RX on arduino TX on DFPlayer
const uint8_t DFPlayer_arduinoTX_pin = 11; //D11 -> TX on arduino RX on DFPlayer


const unsigned int ledBlinkInterval = 1000;

const unsigned int volume = 30; // sound volume 0-30 

const unsigned int shakeSensitivity = 20; // up to 1024. The higher the number, the lower the sensitivity


SoftwareSerial DFPlayerSoftwareSerial(10,11);// RX, TX
DFRobotDFPlayerMini mp3Player;

void soundOn(void* params);
void soundOff();
void flapStart();
void flapEnd();
void birdOutStart();
void birdOutEnd();

struct SoundParams {
  int soundId;
  bool triggerBird;
};

SoundParams defaultSoundParams = {1, true};
JobManager sound(550, 2000, soundOn, &defaultSoundParams, soundOff, false);
JobManager flap(500, 1000, flapStart, flapEnd);
JobManager birdOut(300, 5000, birdOutStart, birdOutEnd);

struct FlapParams {
  int amount;
};

void soapOn() {
  Serial.println("switch soap on");
  digitalWrite(pump_pin, LOW);

  SoundParams soundParams = {1,true};
  //sound.setNewBackoffTime(123123); //lookup mp3 length
  sound.startJob(&soundParams);
}

void soapOff() {
  Serial.println("switch soap off");
  digitalWrite(pump_pin, HIGH);
}

void roomOn() {
}

void roomOff() {
}

void shakeOn() {
}

void shakeOff() {
}

void soundOn(void* params) {
  SoundParams* soundParams = static_cast<SoundParams*>(params);
  Serial.println("sound on.");

  mp3Player.play(soundParams->soundId);

  if(soundParams->triggerBird) {
    Serial.println("Also doing Birdstuff");
    birdOut.startJob();
  }

}

void soundOff() {
}

void birdOutStart() {
  Serial.println("Bird out start");
  digitalWrite(birdMotor1Gnd_pin, HIGH);
  digitalWrite(birdMotor1Vcc_pin, LOW);

}

void birdOutEnd() {
  Serial.println("Bird out end");
  digitalWrite(birdMotor1Gnd_pin, LOW);
  digitalWrite(birdMotor1Vcc_pin, HIGH);
  flap.startJob();
}

void flapStart() {
  Serial.println("flap Start");
  digitalWrite(birdFlap_pin, LOW);

}

void flapEnd() {
  Serial.println("flap End");
  digitalWrite(birdFlap_pin, HIGH);
}


JobManager soap(550, 2000, soapOn, soapOff, true);
JobManager room(550, 2000, roomOn, roomOff, true);
JobManager shake(550, 2000, shakeOn, shakeOff);





void setup() {
  pinMode(hand_pin, INPUT);
  pinMode(room_pin, INPUT);
  pinMode(shake_pin, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_pin, OUTPUT);

  pinMode(pump_pin, OUTPUT);
  digitalWrite(pump_pin, HIGH);

  pinMode(birdFlap_pin, OUTPUT);
  digitalWrite(birdFlap_pin, HIGH);

  pinMode(birdMotor1Gnd_pin, OUTPUT);
  digitalWrite(birdMotor1Gnd_pin, LOW);
  pinMode(birdMotor1Vcc_pin, OUTPUT);
  digitalWrite(birdMotor1Vcc_pin, HIGH);
  pinMode(birdMotor2Gnd_pin, OUTPUT);
  digitalWrite(birdMotor2Gnd_pin, LOW);
  pinMode(birdMotor2Vcc_pin, OUTPUT);
  digitalWrite(birdMotor2Vcc_pin, HIGH);
 
  
  DFPlayerSoftwareSerial.begin(9600);
  Serial.begin(9600); // DFPlayer Mini mit SoftwareSerial initialisieren
  mp3Player.begin(DFPlayerSoftwareSerial, false, true);
  mp3Player.volume(volume); // Lautstärke auf 20 (0-30) setzen

}


void loop() {
  delay(5);

  // Sensor-Zustand überprüfen
  bool handSensor_isOn = !digitalRead(hand_pin);
  bool roomSensor_isOn = digitalRead(room_pin);
  int shakeSensor_value = analogRead(shake_pin);

  soap.handleJob();
  room.handleJob();
  shake.handleJob();
  sound.handleJob();
  birdOut.handleJob();
  flap.handleJob();

  //--------------------------------------
  if (handSensor_isOn) {
    soap.startJob();
  } else {
    soap.resetJob();
  }

  if (roomSensor_isOn) {
    room.startJob();
  }

  if(shakeSensor_value > 20) {
    shake.startJob();
  }
  //--------------------------------------

  //display sensor1 state with buldin led
  digitalWrite(LED_BUILTIN, handSensor_isOn);

}
