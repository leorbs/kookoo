#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "JobManager.cpp"
#include "TimeBasedCounter.cpp"
#include "BirdFlapGenerator.h"

// comment out this line, if you want to show logs on serial:
// #define NDEBUG


#define MAIN_LOOP_TIME_BASE_MS	5

#define HAND_PIN A0            // connect IR hand sensor module to Arduino pin A0
#define ROOM_PIN A1            // praesense sensor module to Arduino pin A1
#define SHAKE_PIN A2           // sensor module for tamper detection to Arduino pin A2
#define RNG_SEED_PIN A3        // Used to initialize the randomnes A3

#define LED1_PIN 9             // D9 LED
#define PUMP_PIN 7             // D7 soap pump

#define BIRD_FLAP_PIN 6        // D6 bird flap

#define BIRD_MOTOR1_GND_PIN 5  // D5 
#define BIRD_MOTOR1_VCC_PIN 2  // D2

#define BIRD_MOTOR2_GND_PIN 3  // D3 
#define BIRD_MOTOR2_VCC_PIN 4  // D4

#define DFPLAYER_RX_PIN 10     // D10 -> RX on Arduino TX on DFPlayer
#define DFPLAYER_TX_PIN 11     // D11 -> TX on Arduino RX on DFPlayer

#define VOLUME 23              // sound volume 0-30
#define SHAKE_SENSITIVITY 20   // up to 1024. The higher the number, the lower the sensitivity

#define LED_BRIGHTNESS 150      // 0-255


SoftwareSerial DFPlayerSoftwareSerial(DFPLAYER_RX_PIN,DFPLAYER_TX_PIN);// RX, TX
DFRobotDFPlayerMini mp3Player;
SoftTimer mainLoopTimer;

#define FOLDER_STANDARD_BIRD_SOUND 1
#define FOLDER_SHAKE_SENSOR_ACTIVATED 2
#define FOLDER_ROOM_START 3
#define FOLDER_ROOM_END 13

const uint8_t maxFolders = FOLDER_ROOM_END;
uint8_t folderFileCounts[maxFolders + 1]; // Index 1-13 used
uint8_t currentRoomFolder = FOLDER_ROOM_START;

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


const uint16_t flapBreakPattern_single[] = {200, 600};
const uint16_t flapPattern_single[] =        {500};

SoundParams soundParams;

int flapPattern_currentIndex = 0;
int flapBreakPattern_currentIndex = 0;

JobManager sound(15000, 400, soundOn, soundOff, false, false); //backoff is the safety time for bird termination

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
  Serial.println(F("switch soap on"));
  digitalWrite(PUMP_PIN, LOW);

  // set new sound params for the sound job TEST
  Serial.print(sound.isJobActive());
  Serial.println(F(" is the isJobActive"));

  if(!sound.isJobActive()) {

    soundParams.folderId = FOLDER_STANDARD_BIRD_SOUND;
    soundParams.triggerBird = true;
    Serial.print(F(" setting soundParams.flapBreakPattern. flapBreakPattern_single[0]: "));
    Serial.println(flapBreakPattern_single[0]);
    soundParams.flapBreakPattern = flapBreakPattern_single;
    soundParams.flapPattern = flapPattern_single;
    soundParams.flapBreakPatternSize = 2;
    soundParams.flapPatternSize = 1;

    sound.startJob();
  }

}

void soapOff() {
  Serial.println(F("switch soap off"));
  digitalWrite(PUMP_PIN, HIGH);
}

void roomOn() {

  Serial.println(F("room On"));

  if(!sound.isJobActive()) {
    //execute the chain
    generateSpeechLikeFlappingPattern(soundParams);
    soundParams.folderId = currentRoomFolder;
    soundParams.triggerBird = true;

    sound.startJob();
  }
}

void roomOff() {
}

void shakeOn() {
  Serial.println(F("shake On"));
  if(!sound.isJobActive()) {

    // flaps get ignored because trigger bird is false
    soundParams.folderId = FOLDER_SHAKE_SENSOR_ACTIVATED;
    soundParams.triggerBird = false;
    soundParams.flapBreakPattern = flapBreakPattern_single;
    soundParams.flapPattern = flapPattern_single;
    soundParams.flapBreakPatternSize = 2;
    soundParams.flapPatternSize = 1;

    //execute the chain
    sound.startJob();
  }
}

void shakeOff() {
}

void soundOn() {

  Serial.println(F("sound on."));
  Serial.print(soundParams.folderId);
  Serial.println(F(" folderId playing"));

  
  int count = folderFileCounts[soundParams.folderId];
  uint8_t fileNum = random(1, count + 1);
  Serial.print(F("Playing file "));
  Serial.print(fileNum);
  Serial.print(F(" from folder "));
  Serial.println(soundParams.folderId);
  mp3Player.playFolder(soundParams.folderId, fileNum);

  if(soundParams.triggerBird) {
    Serial.println(F("Sound doing Birdstuff"));

    // 15 Seconds max sound lengh for safety
    // some other routine should test if mp3 is still playing and  stop sound job (and therefore pull in bird)
    sound.setNewDurationTime(15000);
    sound.restartJobTimer();

    flapPattern_currentIndex = 0;
    flapBreakPattern_currentIndex = 0;

    //execute the bird chain
    birdOut.startJob();
  }

}

void soundOff() {
}

void birdOutStart() {
  Serial.println(F("Bird out start"));
  digitalWrite(BIRD_MOTOR1_GND_PIN, HIGH);
  digitalWrite(BIRD_MOTOR1_VCC_PIN, LOW);
}

void birdOutEnd() {
  Serial.println(F("Bird out end"));
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
  Serial.println(F("Bird in start"));
  digitalWrite(BIRD_MOTOR2_GND_PIN, HIGH);
  digitalWrite(BIRD_MOTOR2_VCC_PIN, LOW);

}

void birdInEnd() {
  Serial.println(F("Bird in end"));
  digitalWrite(BIRD_MOTOR2_GND_PIN, LOW);
  digitalWrite(BIRD_MOTOR2_VCC_PIN, HIGH);
}


// ========================================================================================================================

void flapStart() {
  Serial.print(F("flap Start"));
  Serial.println(soundParams.flapPattern[flapPattern_currentIndex]);

  digitalWrite(BIRD_FLAP_PIN, LOW);

  flap.setNewDurationTime(soundParams.flapPattern[flapPattern_currentIndex]);
  flap.restartJobTimer();
}

void flapEnd() {
  Serial.println(F("flap End"));
  digitalWrite(BIRD_FLAP_PIN, HIGH);

  flapPattern_currentIndex = flapPattern_currentIndex + 1;

  //check flap break possible
  if(flapBreakPattern_currentIndex < soundParams.flapBreakPatternSize && !sound.isBackoffActive()) {
    flapBreak.startJob();
  } else {
    //no flap break anymore (or sound got ended). Do now bird in
    birdIn.startJob();
  }

}

void flapBreakStart(){
  Serial.print(F("flapBreakStart "));
  Serial.println(soundParams.flapBreakPattern[flapBreakPattern_currentIndex]);

  flapBreak.setNewDurationTime(soundParams.flapBreakPattern[flapBreakPattern_currentIndex]);
  flapBreak.restartJobTimer();

}

void flapBreakEnd() {
  Serial.println(F("flapBreakEnd"));

  flapBreakPattern_currentIndex = flapBreakPattern_currentIndex + 1;


  //check flap possible
  if(flapPattern_currentIndex < soundParams.flapPatternSize && !sound.isBackoffActive()) {
    flap.startJob();
  } else {
    //no flap anymore (or sound got ended). Do now bird in
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


  //initialize randomness so it is not every time the same
  randomSeed(analogRead(RNG_SEED_PIN));

  #ifdef NDEBUG
    Serial.begin(9600);
  #endif

  //mp3 player stuff
  DFPlayerSoftwareSerial.begin(9600); // DFPlayer Mini mit SoftwareSerial initialisieren
  mp3Player.begin(DFPlayerSoftwareSerial, true, false);
  mp3Player.setTimeOut(2000);


  mp3Player.volume(0);
  delay(50);

  int consecutiveSame = 0;
  int lastValue = INT16_MAX;
  int currentRead;

  while(consecutiveSame < 3) {
    Serial.print(F("consecutiveSame: "));
    Serial.println(consecutiveSame);
    currentRead = mp3Player.readFolderCounts();
    Serial.print(F("currentRead: "));
    Serial.println(currentRead);
    if(lastValue == currentRead){
      consecutiveSame += 1;
    } else {
      consecutiveSame = 0;
      lastValue = currentRead;
    }
  }

  int maxDetectedFolders = lastValue;
  Serial.print(F("maxDetectedFolders "));
  Serial.println(maxDetectedFolders);

  if(maxDetectedFolders > maxFolders) {
    maxDetectedFolders = maxFolders;
  }

  for (int i = 1; i <= maxDetectedFolders ; i++) {

    mp3Player.playFolder(i, 1);
    delay(100);

    lastValue = INT16_MAX;
    consecutiveSame = 0;
    while(consecutiveSame < 3) {
      Serial.print(F("consecutiveSame: "));
      Serial.println(consecutiveSame);
      currentRead = mp3Player.readFileCountsInFolder(0);
      delay(20);
      Serial.print(F("currentRead: "));
      Serial.println(currentRead);
      if(lastValue == currentRead){
        consecutiveSame += 1;
      } else {
        consecutiveSame = 0;
        lastValue = currentRead;
      }
    }

    folderFileCounts[i] = lastValue;

    Serial.print(F("folderFileCounts[i] i: "));
    Serial.print(i);
    Serial.print(F(" folderFileCounts: "));
    Serial.println(lastValue);
  }


  mp3Player.stop();
  delay(500);

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
  uint16_t shakeSensor_value = analogRead(SHAKE_PIN);

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
  // sound observation loop

  if(sound.isJobActive() && mp3Player.available() ) {

    uint8_t type = mp3Player.readType();
    int value = mp3Player.read();


    if (type == DFPlayerPlayFinished) {
    //sound is done playing. Terminate Bird.
      Serial.print(F("Finished playing file "));
      Serial.println(value);
      sound.endJob(); //terminates bird
    }
  }


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

    // Serial.println(F("================"));
    // Serial.print(wereThereMultipleShakeIncidents);
    // Serial.println(F(" = wereThereMultipleShakeIncidents"));
    // Serial.print(timeBasedCounter.getLatestTime());
    // Serial.println(F(" = latest time"));
    // Serial.print(currentTime);
    // Serial.println(F(" = current time"));

    // Serial.print("shake detected: shake value ");
    // Serial.println(shakeSensor_value);
    // Serial.print(timeBasedCounter.getCurrentShakeCounter(currentTime));
    // Serial.println(F(" is the current shakes detected"));
    // Serial.println(F("================"));


    if(wereThereMultipleShakeIncidents) {
      shake.startJob();
    }
  }

}

void ledOnStart() {
  // Serial.println(F("LED on"));
  analogWrite(LED1_PIN, LED_BRIGHTNESS);
}
void ledOnEnd() {
  ledOff.startJob();
}
void ledOffStart() {
  // Serial.println(F("LED off"));
  digitalWrite(LED1_PIN, LOW);
}
void ledOffEnd() {
  ledOn.startJob();
}
