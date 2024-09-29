
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "JobManager.cpp"


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

const uint8_t DFPlayerRX_pin = 10; //D10 -> RX on arduino TX on DFPlayer
const uint8_t DFPlayerTX_pin = 11; //D11 -> TX on arduino RX on DFPlayer


const unsigned int ledBlinkInterval = 1000;

const unsigned int volume = 30; // sound volume 0-30 

const unsigned int shakeSensitivity = 20; // up to 1024. The higher the number, the lower the sensitivity


SoftwareSerial DFPlayerSoftwareSerial(DFPlayerRX_pin,DFPlayerTX_pin);// RX, TX
DFRobotDFPlayerMini mp3Player;

void soundOn();
void soundOff();
void flapStart();
void flapEnd();
void birdOutStart();
void birdOutEnd();
void birdInStart();
void birdInEnd();
void flapBreakStart();
void flapBreakEnd();
void soapOn();
void soapOff();
void roomOn();
void roomOff();
void shakeOn();
void shakeOff();

struct SoundParams {
  int soundId;
  bool triggerBird;
};

struct FlapParams {
  int amount;
};

struct FlapBreakParams {
  int amount;
};

SoundParams soundParams = {1, true};
FlapParams flapParams = {1};
FlapBreakParams flapBreakParams = {2};

JobManager sound(550, 2000, soundOn, soundOff, false);

JobManager flap(500, 1, flapStart, flapEnd);
JobManager flapBreak(550, 1, flapBreakStart, flapBreakEnd);
JobManager birdOut(240, 1, birdOutStart, birdOutEnd);
JobManager birdIn(260, 1, birdInStart, birdInEnd);

JobManager soap(550, 2000, soapOn, soapOff, true);
JobManager room(550, 2000, roomOn, roomOff, true);
JobManager shake(550, 2000, shakeOn, shakeOff);

// only sound will control bird
// bird can only come out with sound
// that means if one sound is running, there can be no other sound and therefore no other bird
// sound execution time -> full bird cycle. Execution time mandatory to prevent double bird scenario



void soapOn() {
  Serial.println("switch soap on");
  digitalWrite(pump_pin, LOW);

  // set new sound params for the sound job TEST
  Serial.print(sound.isJobActive());
  Serial.println(" is the isJobActive");

  Serial.print(sound.isBackoffActive());
  Serial.println(" is the isBackoffActive");

  if(!sound.isJobActive() && !sound.isBackoffActive()) {
    //setup the bird chain
    flapBreakParams.amount=2;
    flapParams.amount=1;
    soundParams = {1,true};

    uint32_t birdCycleTime = 
    birdOut.getJobDuration() + 
    (flapBreakParams.amount * flapBreak.getJobDuration()) + 
    (flapParams.amount * flap.getJobDuration()) +
    birdIn.getJobDuration();

    birdCycleTime = birdCycleTime * 1.2f; //a bit more

    Serial.print(birdCycleTime);
    Serial.println(" is the birdCycleTime");

    sound.setNewDurationTime(birdCycleTime);
    sound.startJob();
  }

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

void soundOn() {

  Serial.println("sound on.");
  mp3Player.play(soundParams.soundId);

  if(soundParams.triggerBird) {
    Serial.println("Sound doing Birdstuff");

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
  
  //execute
  if(flapBreakParams.amount > 0) {
    flapBreak.startJob();
  } else {
    birdIn.startJob();
  }

}

void birdInStart() {
  Serial.println("Bird in start");
  digitalWrite(birdMotor2Gnd_pin, HIGH);
  digitalWrite(birdMotor2Vcc_pin, LOW);

}

void birdInEnd() {
  Serial.println("Bird in end");
  digitalWrite(birdMotor2Gnd_pin, LOW);
  digitalWrite(birdMotor2Vcc_pin, HIGH);
}

void flapStart() {
  Serial.println("flap Start");
  digitalWrite(birdFlap_pin, LOW);
  // analogWrite(birdFlap_pin, 50);

}

void flapEnd() {
  Serial.println("flap End");
  digitalWrite(birdFlap_pin, HIGH);

  flapParams.amount = flapParams.amount - 1;

  if(flapBreakParams.amount > 0) {
    flapBreak.startJob();
  } else {
    birdIn.startJob();
  }
}

void flapBreakStart(){
  Serial.println("flapBreakStart");

}

void flapBreakEnd() {
  Serial.println("flapBreakEnd");

  flapBreakParams.amount = flapBreakParams.amount - 1;

  if(flapParams.amount > 0) {
    flap.startJob();
  } else {
    birdIn.startJob();
  }
}


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
  birdIn.handleJob();
  flap.handleJob();
  flapBreak.handleJob();


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
