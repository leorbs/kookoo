#include "Arduino.h"
#include <Bounce2.h> 


#define BUTTON_1 12




Bounce2::Button button = Bounce2::Button(); // INSTANTIATE A Bounce2::Button OBJECT


void setup() {

  button.attach ( BUTTON_1 , INPUT_PULLUP );
  button.interval( 10 );
  button.setPressedState( LOW ); 

  Serial.begin(9600);

}

void loop() {

  // UPDATE THE BUTTON BY CALLING .update() AT THE BEGINNING OF THE LOOP:
  button.update();

  // IF THE BUTTON WAS PRESSED THIS LOOP:
  if ( button.pressed() ) {
    Serial.println("button pressed");
  }

}

