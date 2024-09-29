
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "JobManager.cpp"
#include "TimeBasedCounter.cpp"


#define MAIN_LOOP_TIME_BASE_MS	5

#define HAND_PIN A0            // connect IR hand sensor module to Arduino pin A0
#define ROOM_PIN A1            // praesense sensor module to Arduino pin A1
#define SHAKE_PIN A2           // sensor module for tamper detection to Arduino pin A2

#define LED1_PIN 9             // D9 LED
#define PUMP_PIN 7             // D7 soap pump

#define BIRD_FLAP_PIN 6        // D6 bird flap

#define BIRD_MOTOR1_GND_PIN 5  // D5 
#define BIRD_MOTOR1_VCC_PIN 2  // D2

#define BIRD_MOTOR2_GND_PIN 3  // D3 
#define BIRD_MOTOR2_VCC_PIN 4  // D4

#define DFPLAYER_RX_PIN 10     // D10 -> RX on Arduino TX on DFPlayer
#define DFPLAYER_TX_PIN 11     // D11 -> TX on Arduino RX on DFPlayer

#define VOLUME 5               // sound volume 0-30
#define SHAKE_SENSITIVITY 20   // up to 1024. The higher the number, the lower the sensitivity


SoftwareSerial DFPlayerSoftwareSerial(DFPLAYER_RX_PIN,DFPLAYER_TX_PIN);// RX, TX
DFRobotDFPlayerMini mp3Player;
SoftTimer mainLoopTimer;

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
void ledOnStart();
void ledOnEnd();
void ledOffStart();
void ledOffEnd();

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

JobManager sound(550, soundOn, soundOff, false);

JobManager flap(500, flapStart, flapEnd);
JobManager flapBreak(200, flapBreakStart, flapBreakEnd);
JobManager birdOut(240, birdOutStart, birdOutEnd);
JobManager birdIn(260, birdInStart, birdInEnd);

JobManager soap(550, 2000, soapOn, soapOff, true);
JobManager room(500, 60000, roomOn, roomOff, true, true);
JobManager shake(550, 10000, shakeOn, shakeOff);

// only sound will control bird
// bird can only come out with sound
// that means if one sound is running, there can be no other sound and therefore no other bird
// sound execution time -> full bird cycle. Execution time mandatory to prevent double bird scenario

JobManager ledOn(500, ledOnStart, ledOnEnd);
JobManager ledOff(500, ledOffStart, ledOffEnd);


void soapOn() {
  Serial.println("switch soap on");
  digitalWrite(PUMP_PIN, LOW);

  // set new sound params for the sound job TEST
  Serial.print(sound.isJobActive());
  Serial.println(" is the isJobActive");

  if(!sound.isJobActive()) {
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
    //execute the chain
    sound.startJob();
  }

}

void soapOff() {
  Serial.println("switch soap off");
  digitalWrite(PUMP_PIN, HIGH);
}

void roomOn() {

  Serial.println("room On");

  if(!sound.isJobActive()) {
    //setup the room chain
    flapBreakParams.amount=5;
    flapParams.amount=4;
    soundParams = {2,true};

    uint32_t birdCycleTime = 
    birdOut.getJobDuration() + 
    (flapBreakParams.amount * flapBreak.getJobDuration()) + 
    (flapParams.amount * flap.getJobDuration()) +
    birdIn.getJobDuration();

    birdCycleTime = birdCycleTime * 1.2f; //a bit more

    Serial.print(birdCycleTime);
    Serial.println(" is the birdCycleTime");

    sound.setNewDurationTime(birdCycleTime);
    //execute the chain
    sound.startJob();
  }
}

void roomOff() {
}

void shakeOn() {
  Serial.println("shake On");
  if(!sound.isJobActive()) {

    soundParams = {3,false};

    sound.setNewDurationTime(5000);
    //execute the chain
    sound.startJob();
  }
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
  digitalWrite(BIRD_MOTOR1_GND_PIN, HIGH);
  digitalWrite(BIRD_MOTOR1_VCC_PIN, LOW);
}

void birdOutEnd() {
  Serial.println("Bird out end");
  digitalWrite(BIRD_MOTOR1_GND_PIN, LOW);
  digitalWrite(BIRD_MOTOR1_VCC_PIN, HIGH);
  
  //execute the chain
  if(flapBreakParams.amount > 0) {
    flapBreak.startJob();
  } else {
    birdIn.startJob();
  }

}

void birdInStart() {
  Serial.println("Bird in start");
  digitalWrite(BIRD_MOTOR2_GND_PIN, HIGH);
  digitalWrite(BIRD_MOTOR2_VCC_PIN, LOW);

}

void birdInEnd() {
  Serial.println("Bird in end");
  digitalWrite(BIRD_MOTOR2_GND_PIN, LOW);
  digitalWrite(BIRD_MOTOR2_VCC_PIN, HIGH);
}

void flapStart() {
  Serial.println("flap Start");
  digitalWrite(BIRD_FLAP_PIN, LOW);
  // analogWrite(birdFlap_pin, 50);

}

void flapEnd() {
  Serial.println("flap End");
  digitalWrite(BIRD_FLAP_PIN, HIGH);

  flapParams.amount = flapParams.amount - 1;

  //execute the chain
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

  //execute the chain
  if(flapParams.amount > 0) {
    flap.startJob();
  } else {
    birdIn.startJob();
  }
}


void setup() {
  mainLoopTimer.setTimeOutTime(MAIN_LOOP_TIME_BASE_MS);
  mainLoopTimer.reset();

  pinMode(HAND_PIN, INPUT);
  pinMode(ROOM_PIN, INPUT);
  pinMode(SHAKE_PIN, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);

  pinMode(BIRD_FLAP_PIN, OUTPUT);
  digitalWrite(BIRD_FLAP_PIN, HIGH);

  pinMode(BIRD_MOTOR1_GND_PIN, OUTPUT);
  digitalWrite(BIRD_MOTOR1_GND_PIN, LOW);
  pinMode(BIRD_MOTOR1_VCC_PIN, OUTPUT);
  digitalWrite(BIRD_MOTOR1_VCC_PIN, HIGH);
  pinMode(BIRD_MOTOR2_GND_PIN, OUTPUT);
  digitalWrite(BIRD_MOTOR2_GND_PIN, LOW);
  pinMode(BIRD_MOTOR2_VCC_PIN, OUTPUT);
  digitalWrite(BIRD_MOTOR2_VCC_PIN, HIGH);
 
  
  DFPlayerSoftwareSerial.begin(9600);
  Serial.begin(9600); // DFPlayer Mini mit SoftwareSerial initialisieren
  mp3Player.begin(DFPlayerSoftwareSerial, false, true);
  mp3Player.volume(VOLUME); // Lautstärke auf 20 (0-30) setzen

}

TimeBasedCounter timeBasedCounter;
unsigned long currentTime = 0;

void loop() {
  while(!mainLoopTimer.hasTimedOut());
  mainLoopTimer.reset();
  
  currentTime = millis();

  // Sensor-Zustand überprüfen
  bool handSensor_isOn = !digitalRead(HAND_PIN);
  bool roomSensor_isOn = digitalRead(ROOM_PIN);
  int shakeSensor_value = analogRead(SHAKE_PIN);

  //display sensor1 state with buldin led
  digitalWrite(LED_BUILTIN, handSensor_isOn);

  soap.handleJob();
  room.handleJob();
  shake.handleJob();
  sound.handleJob();
  birdOut.handleJob();
  birdIn.handleJob();
  flap.handleJob();
  flapBreak.handleJob();
  ledOn.handleJob();
  ledOff.handleJob();


  //--------------------------------------
  if (handSensor_isOn) {
    soap.startJob();
    timeBasedCounter.reset(); //shake is allowed when there is normal usage
    room.renewBackoff(); // when someone uses the soap, we dont need to execute the room procedure
  } else {
    soap.resetJob();
  }

  if (roomSensor_isOn) {
    room.startJob();
    room.renewBackoff(); // this renewes the backoff when someone is constantly in the room. The "room experience" is only for when you enter the room
  } else {
    room.resetJob();
  }
  

  if(shakeSensor_value > SHAKE_SENSITIVITY && currentTime > timeBasedCounter.getLatestTime() + 1000 && !shake.isBackoffActive()) {

    bool wereThereMultipleShakeIncidents = timeBasedCounter.addTimeAndCheck(currentTime);

    // Serial.println("================");
    // Serial.print(wereThereMultipleShakeIncidents);
    // Serial.println(" = wereThereMultipleShakeIncidents");
    // Serial.print(timeBasedCounter.getLatestTime());
    // Serial.println(" = latest time");
    // Serial.print(currentTime);
    // Serial.println(" = current time");

    // Serial.print("shake detected: shake value ");
    // Serial.println(shakeSensor_value);
    // Serial.print(timeBasedCounter.getCurrentShakeCounter(currentTime));
    // Serial.println(" is the current shakes detected");
    // Serial.println("================");


    if(wereThereMultipleShakeIncidents) {
      shake.startJob();
    }
  }

}

void ledOnStart() {
  analogWrite(LED1_PIN, 128);
}
void ledOnEnd() {
  ledOff.startJob();
}
void ledOffStart() {
  digitalWrite(LED1_PIN, LOW);
}
void ledOffEnd() {
  ledOn.startJob();
}
