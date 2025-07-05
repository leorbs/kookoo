
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

#define VOLUME 6               // sound volume 0-30
#define SHAKE_SENSITIVITY 20   // up to 1024. The higher the number, the lower the sensitivity

#define LED_BRIGHTNESS 80     // 0-255


SoftwareSerial DFPlayerSoftwareSerial(DFPLAYER_RX_PIN,DFPLAYER_TX_PIN);// RX, TX
DFRobotDFPlayerMini mp3Player;
SoftTimer mainLoopTimer;

TimeBasedCounter timeBasedCounter;
unsigned long currentTime = 0;

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
  uint32_t* flapBreakPattern;  // Pointer to an array of uint32_t
  uint32_t* flapPattern;       // Pointer to an array of uint32_t
  int flapBreakPatternSize;    // Size of the flapBreakPattern array
  int flapPatternSize;         // Size of the flapPattern array
};

struct FlapParams {
  int amount;
};

struct FlapBreakParams {
  int amount;
};

uint32_t flapBreakPattern_single[2] = {200,600};
uint32_t flapPattern_single[1] =        {500};

uint32_t flapBreakPattern_speak_pattern_1[7] = {500,400,300,800,400,500,1000};
uint32_t flapPattern_speak_pattern_1[6] =        {100,50,  50, 600,50,1000};

SoundParams soundParams = {1, true, flapBreakPattern_single, flapPattern_single, 2, 1};

uint8_t flapPattern_index = 0;
uint8_t flapBreakPattern_index = 0;

JobManager sound(550, soundOn, soundOff, false);

JobManager flap(500, flapStart, flapEnd);
JobManager flapBreak(200, flapBreakStart, flapBreakEnd);
JobManager birdOut(240, birdOutStart, birdOutEnd);
JobManager birdIn(260, birdInStart, birdInEnd);

JobManager soap(400, 2000, soapOn, soapOff, true);
JobManager room(500, 60000, roomOn, roomOff, true, true);
JobManager shake(550, 10000, shakeOn, shakeOff, false, true);

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

    soundParams = {1, true, flapBreakPattern_single, flapPattern_single, 2, 1};
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
    //execute the chain
    soundParams = {2, true, flapBreakPattern_speak_pattern_1, flapPattern_speak_pattern_1, 7, 6};
    sound.startJob();
  }
}

void roomOff() {
}

void shakeOn() {
  Serial.println("shake On");
  if(!sound.isJobActive()) {

    // ignore after false
    soundParams = {3, false, flapBreakPattern_single, flapPattern_single, 2, 1};

    sound.setNewDurationTime(5000);
    //execute the chain
    sound.startJob();
  }
}

void shakeOff() {
}

void soundOn() {

  Serial.println("sound on.");
  Serial.print(soundParams.soundId);
  Serial.println(" soundid playing");

  mp3Player.play(soundParams.soundId);

  if(soundParams.triggerBird) {
    Serial.println("Sound doing Birdstuff");

    //setup the bird chain
    uint32_t birdCycleTime = 0;

    // Add all values from flapBreakPattern array
    for (int i = 0; i < soundParams.flapBreakPatternSize; ++i) {
        birdCycleTime += soundParams.flapBreakPattern[i];
    }

    // Add all values from flapPattern array
    for (int i = 0; i < soundParams.flapPatternSize; ++i) {
        birdCycleTime += soundParams.flapPattern[i];
    }

    birdCycleTime = birdCycleTime + birdOut.getJobDuration() + birdIn.getJobDuration();

    birdCycleTime = birdCycleTime + 50; //a bit more

    Serial.print(birdCycleTime);
    Serial.println(" is the birdCycleTime");

    sound.setNewDurationTime(birdCycleTime);
    sound.restartJobTimer();

    flapPattern_index = 0;
    flapBreakPattern_index = 0;

    //execute the bird chain
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
  if(soundParams.flapBreakPatternSize > soundParams.flapPatternSize) {
    flapBreak.startJob();
  } else {
    flap.startJob();
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


// ========================================================================================================================

void flapStart() {
  Serial.println("flap Start");
  digitalWrite(BIRD_FLAP_PIN, LOW);

  flap.setNewDurationTime(soundParams.flapPattern[flapPattern_index]);
  flap.restartJobTimer();
}

void flapEnd() {
  Serial.println("flap End");
  digitalWrite(BIRD_FLAP_PIN, HIGH);

  flapPattern_index = flapPattern_index + 1;

  //check flap break possible
  if(flapBreakPattern_index < soundParams.flapBreakPatternSize) {
    flapBreak.startJob();
  } else {
    //no flap break anymore. Do now bird in
    birdIn.startJob();
  }

}

void flapBreakStart(){
  Serial.println("flapBreakStart");

  flapBreak.setNewDurationTime(soundParams.flapBreakPattern[flapBreakPattern_index]);
  flapBreak.restartJobTimer();

}

void flapBreakEnd() {
  Serial.println("flapBreakEnd");

  flapBreakPattern_index = flapBreakPattern_index + 1;


  //check flap possible
  if(flapPattern_index < soundParams.flapPatternSize) {
    flap.startJob();
  } else {
    //no flap anymore. Do now bird in
    birdIn.startJob();
  }

}
// ========================================================================================================================

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
  mp3Player.volume(VOLUME);

  ledOn.startJob();
}

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

  // new idea:
  // add the analog signal to a counter
  // decrease counter in each loop
  // if counter too high, then tilt
  

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
  Serial.println("LED on");
  analogWrite(LED1_PIN, LED_BRIGHTNESS);
}
void ledOnEnd() {
  ledOff.startJob();
}
void ledOffStart() {
  Serial.println("LED off");
  digitalWrite(LED1_PIN, LOW);
}
void ledOffEnd() {
  ledOn.startJob();
}
