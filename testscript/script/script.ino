
#include "Arduino.h"
#include "SoftwareSerial.h"

const int sensor1_pin = A7;

const int sensor2_pin = A5;
const int sensor3_pin = A4;

int val = 0;

void setup() {
  Serial.begin(9600);           //  setup serial
  pinMode(sensor3_pin, INPUT);    
}

void loop() {
  delay(10);

//  val = analogRead(sensor2_pin);  // read the input pin


  val = digitalRead(sensor3_pin);

  if( val == 1)
    Serial.println(val);          // debug value
  else 
    Serial.println(" ");
}
