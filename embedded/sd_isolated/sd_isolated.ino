#include <SD.h>

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(5000);

  Serial.print("Initializing SD card...");

  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1){
      Serial.println("fuck");
    }
  }
  Serial.println("initialization done.");

  myFile = SD.open("/shit.txt", FILE_APPEND);
  if (myFile){
    myFile.println("BRUH");
    myFile.println("SHIT");
    Serial.println("Wrote to shit.txt");
  } else {
    SD.mkdir("/shit.txt");
    myFile = SD.open("/shit.txt");
    
    Serial.println("Created shit.txt and wrote to it");
  }
  myFile.close();

  if (SD.exists("/shit.txt")){
    myFile = SD.open("/shit.txt", FILE_READ);
    Serial.println("Reading from shit.txt");
    /*
    char buf[20];

    int rlen = myFile.available();
    char ch = myFile.read();
    myFile.read((uint8_t)atoi(buf), rlen - 1);
    Serial.print(ch);
    Serial.print(buf);
    */
    char character = myFile.read();
    while (character != NULL){
      Serial.print(character);
      character = myFile.read();
    }
    
  myFile.close();
  } else {
    Serial.println("Could not be read.");
  }
}

void loop() {
  
}
