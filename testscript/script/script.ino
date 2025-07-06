#include <DFPlayerMini_Fast.h>


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX


DFPlayerMini_Fast myMP3;

void setup()
{
  Serial.begin(9600);


  mySerial.begin(9600);
  myMP3.begin(mySerial, true, 5000);

  mySerial.listen();
  myMP3.setTimeout(5000);

  
  delay(1000);
  
  Serial.println("Setting volume to max");
  myMP3.volume(20);

  for (int i = 0; i <= 5; i++) {
    Serial.print(F("Folder "));
    Serial.print(i);
    Serial.print(F(" has "));
    Serial.print(myMP3.numTracksInFolder(i));
    Serial.println(F(" files in it."));
  }
  
  Serial.println("Looping track 1");
  myMP3.loop(1);


}

void loop()
{
  //do nothing
}