#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SD.h>
#include <Servo.h>

#define SEALEVELPRESSURE_HPA (1013.25) // predefined by BMP, change as needed

Adafruit_BMP3XX bmp; // I2C address 0x77
SFE_UBLOX_GNSS myGNSS; // I2C address 0x42
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); // I2C address 0x28

// BMP
float temperature;
float altitude;
float pressure;
float alt_offset;
double last_alt;

// SAM
float latitude;
float longitude;
float gps_altitude;
int siv; // satellites in view
String gps_time;
String gps_hour;
String gps_min;
String gps_sec;

// BNO
float tiltx;
float tilty;

// ADC Pin
float voltage;

// Internal for Packet
String missionTime = "00:00:00";
int packetCount = 0;
char flightMode = 'F';
String flightState = "IDLE";
char hs_deployed = 'N';
char pc_deployed = 'N';
char mast_raised = 'N';
String cmdecho = "NONE";

// Internal for Reset
int startHour;
int startMinute;
int startSecond;
bool containerReleased;

File packet_csv;
Servo release_servo;
Servo flag_servo;
Servo hs_servo;


///////////////////////////////////// SETUP /////////////////////////////////////
void setup() {

  Serial.begin(115200);
  Wire.begin();

  // BMP set up
  if (!bmp.begin_I2C()) {
    while (1){
      Serial.println("BMP NOT FOUND");
    }
  }
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // get rid of bad altitude data
  bmp.readAltitude(SEALEVELPRESSURE_HPA);
  bmp.readAltitude(SEALEVELPRESSURE_HPA);
  bmp.readAltitude(SEALEVELPRESSURE_HPA);

  alt_offset = setDefaultAlt();

  // SAM set up
  if (!myGNSS.begin()){
    while (1){
      Serial.println("SAM NOT FOUND");
    }
  }
  // myGNSS.enableDebugging(); // in case of error
  
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  // BNO set up
  if (!bno.begin()){
    while (1){
      Serial.println("BNO NOT FOUND");
    }
  }
  bno.setExtCrystalUse(true);

  startHour = myGNSS.getHour();
  startMinute = myGNSS.getMinute();
  startSecond = myGNSS.getSecond();

  // SD set up
  if (!SD.begin()){
    while (true){
      Serial.println("SD DID NOT BEGIN PROPERLY");
    }
  }
  SD.remove("/testlaunchdata.csv");
  packet_csv = SD.open("/testlaunchdata.csv", FILE_WRITE);
  if (!packet_csv){
    while (true){
      Serial.println("SD CSV DID NOT OPEN PROPERLY");
    }
  }
  packet_csv.close();

  // Servo set up
  release_servo.attach(25); // may need to change
  hs_servo.attach(26); // need to change
  flag_servo.attach(27); // need to change
}




///////////////////////////////////// HELPER FUNCTIONS /////////////////////////////////////
String itemAt(String packet, int pos){
  // pos is 0 indexed
  int lastpos = -1;
  for (int i = 0; i < pos; i++){
    lastpos = packet.indexOf(',', lastpos + 1);
  }
  return packet.substring(lastpos + 1, packet.indexOf(',', lastpos + 1));
}

void writeToFile(String packet, File file){
  /*
   * Writes the packet to the file specified
   * Returns nothing
   */
  for (int i = 0; i < packet.length(); i++){
    char sentData = packet[i]; 
    file.write(sentData);
  }
  Serial.println("Wrote data to CSV.");
}

// led setup 
void ledBlink(){
  pinMode(27,OUTPUT); // need to change

  pinMode(27,HIGH);
  delay(500);
  pinMode(27,LOW);
  delay(500);
}

// buzzer setup
void buzzer(){
  pinMode(30, OUTPUT); // need to change 

  pinMode(30,HIGH);
  delay(500);
  pinMode(30,LOW);
  delay(500);
}

float setDefaultAlt(){
  /*
   *  Sets altitude offset to current altitude
   *  Returns the new offset
   */
   return (bmp.readAltitude(SEALEVELPRESSURE_HPA)
         + bmp.readAltitude(SEALEVELPRESSURE_HPA)
         + bmp.readAltitude(SEALEVELPRESSURE_HPA)
         + bmp.readAltitude(SEALEVELPRESSURE_HPA)
         + bmp.readAltitude(SEALEVELPRESSURE_HPA)) / 5.0;
}

void debugPrintData(){
  /*
   *  Prints all compiled data
   *  Returns none
   */
  Serial.println("BMP DATA");
  Serial.println("Temperature: " + (String)temperature);
  Serial.println("Altitude: " + (String)altitude);
  Serial.println("Pressure: " + (String)pressure);
  Serial.println("Altitude Offset: " + (String)alt_offset);
  Serial.println();
  Serial.println("SAM DATA");
  Serial.println("Latitude: " + (String)latitude);
  Serial.println("Longitude: " + (String)longitude);
  Serial.println("GPS Altitude: " + (String)gps_altitude);
  Serial.println("SIVs: " + (String)siv);
  Serial.println("GPS Time: " + gps_time);
  Serial.println();
  Serial.println("BNO DATA");
  Serial.println("Tilt X: " + (String)tiltx);
  Serial.println("Tilt Y: " + (String)tilty);
  Serial.println();
  Serial.println("ADC PIN");
  Serial.println("Voltage: " + (String)voltage);

  Serial.println("-------------------------");
}




///////////////////////////////////// RELEASE MECHANISMS /////////////////////////////////////
// initial rocket release
void rocketRelease() {
  release_servo.write(90);
  delay(300);
}

// parachute release
void chuteRelease() {
  release_servo.write(0);
  delay(300);
}

// first shield deploy
void shieldDeployOne() {
  hs_servo.write(2400); // don't know this number yet
  delay(300);
}

void shieldRetract() {
;
  
}


// second shield deploy
void upright() {
  hs_servo.write(180); // don't know this number yet
  delay(300);
}

// flag delpy
void flagDeploy() {
  flag_servo.write(180); // don't know this number yet
  delay(300);
}




///////////////////////////////////// FLIGHT STAGES /////////////////////////////////////

//////////////////////////////////////////////////////////////////////////// NEED HELP ////////////////////////////////////////////////////////////////////////////

// ready to ascending state (0m to 700m)
void stateReady() {
  if (altitude >= 2 && altitude <= 700){
    flightState = "ASCENDING";
  }
}

 
// ascending to descending state (0m to 700m to 500m)
void stateAscending() {
  if (altitude >= 700 && !containerReleased){
    containerReleased = true;
    rocketRelease();
  }
  else {
    flightState = "DESCENDING";
  }
}


// descending to releasing hs (700m to 500m)
void stateDescending() {
  if (altitude <= 500 && containerReleased) {
    flightState = "HSDEPLOYED";
  }
}


// shield_release (500m to 200m)
void stateShieldRelease() {
  if (hs_deployed == 'N') {
    shieldDeployOne();
    hs_deployed = 'P';
  }
  if (altitude <= 200) {
    flightState = "PCDEPLOYED";
  }
}


// chute_release (200m to 0m)
void stateChuteRelease() {
  if (pc_deployed == 'N') {
    chuteRelease();
    pc_deployed = 'C';
  }
  if ((altitude - last_alt) < 1) { // ADD ACCELERATION CONDITION AND CHANGE ALTITUDE PARAMETERS!!
    flightState = "LANDED";
  }
}

// landed
void stateLanded() {
  if (mast_raised == 'N') {
    upright();
    delay(5000);
    flagDeploy();
    delay (5000);
    mast_raised = 'M'; 
    buzzer(); //BUZZER ACTIVATES HERE
  }
}
//////////////////////////////////////////////////////////////////////////// NEED HELP ////////////////////////////////////////////////////////////////////////////



///////////////////////////////////// COMMAND READING /////////////////////////////////////
void readcommands(){
  if (Serial.available()){
    String packet = Serial.readString();
    if (itemAt(packet, 0) == "CMD" && itemAt(packet, 1) == "1070"){
      String cmd = itemAt(packet, 2);
      String cmdarg = itemAt(packet, 3);
      if (cmd == "CX"){
        if (cmdarg == "ON"){
          cmdecho = "CXON";
          flightState = "READY";
        }
        else if (cmdarg == "OFF"){
          cmdecho = "CXOFF";
          flightState = "IDLE";
        }
        else {
          Serial.println("Invalid command received.");
        }
      }
      else if (cmd == "ST"){
        if (cmdarg == "GPS") {
          cmdecho = "STGPS";
          
          gps_hour = myGNSS.getHour();
          gps_min = myGNSS.getMinute();
          gps_sec = myGNSS.getSecond();
          
          int dtime = round(millis()/1000);
  
          int dhours = int(floor(dtime / 3600));
          dtime = dtime - dhours * 3600;
          int dminutes = int(floor(dtime / 60));
          dtime = dtime - dminutes * 60;
          int dseconds = dtime;

          startHour = gps_hour.toInt() - dhours;
          startMinute = gps_min.toInt() - dminutes;
          startSecond = gps_sec.toInt() - dseconds;

          if (startSecond < 0){
            startSecond += 60;
            startMinute--;
          }
          if (startMinute < 0){
            startMinute += 60;
            startHour--;
          }
          if (startHour < 0){
            startHour += 24;
          }          
        }
        else {
          cmdecho = "STCUS";
          // hh:mm:ss
          int newhour = cmdarg.substring(0, 2).toInt();
          int newmin = cmdarg.substring(3, 5).toInt();
          int newsec = cmdarg.substring(6).toInt();
          
          int dtime = round(millis()/1000);
  
          int dhours = int(floor(dtime / 3600));
          dtime = dtime - dhours * 3600;
          int dminutes = int(floor(dtime / 60));
          dtime = dtime - dminutes * 60;
          int dseconds = dtime;

          startHour = newhour - dhours;
          startMinute = newmin - dminutes;
          startSecond = newsec - dseconds;

          if (startSecond < 0){
            startSecond += 60;
            startMinute--;
          }
          if (startMinute < 0){
            startMinute += 60;
            startHour--;
          }
          if (startHour < 0){
            startHour += 24;
          } 
        }
      }
      else if (cmd == "SIM"){
        if (cmdarg = "ENABLE") {
          cmdecho = "SIME";
          
        }
        else if (cmdarg = "DISABLE") {
          cmdecho = "SIMD";
          
        }
        else if (cmdarg = "ACTIVATE") {
          cmdecho = "SIMA";
          
        }
        else {
          Serial.println("Invalid command received.");
        }
      }
      else if (cmd == "SIMP"){
        cmdecho = "SIMP";
      }
      else if (cmd == "CAL"){
        cmdecho = "CAL";
        
        float number1 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number2 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number3 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number4 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number5 = bmp.readAltitude(SEALEVELPRESSURE_HPA);;
        
        alt_offset = (number1 + number2 + number3 + number4 + number5)/5;
      }
      else if (cmd == "ACT"){
        if (cmdarg == "MR") {
          cmdecho = "ACTMR";
          rocketRelease();
        }
        else if (cmdarg == "HS") {
          cmdecho = "ACTHS";
          shieldDeployOne();
        }
        else if (cmdarg == "PC") {
          cmdecho = "ACTPC";
          chuteRelease();
        }
        else if (cmdarg == "AB") {
          cmdecho = "ACTAB";
          buzzer();
        }
        else if (cmdarg == "LED") {
          cmdecho = "ACTLED";
          ledBlink();
        }
        else {
          Serial.println("Invalid command received.");
        }
      }
      else {
        Serial.println("Invalid command received.");
      }
    }
  }
}




///////////////////////////////////// READING DATA /////////////////////////////////////
void updateData() {
  /* 
   *  Updates the internal variables with the newly read data
   *  Returns none
   */

  // BMP
  temperature = round(10 * bmp.temperature)/10.0;
  pressure = round(bmp.pressure / 100.0)/10.0;
  last_alt = altitude;
  altitude = round(10 * (bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset))/10.0;

  // SAM
  latitude = myGNSS.getLatitude()/10000000.0;
  longitude = myGNSS.getLongitude()/10000000.0;
  gps_altitude = myGNSS.getAltitude()/1000.0;
  siv = myGNSS.getSIV();
  gps_time = (String)myGNSS.getHour() + ":" + (String)myGNSS.getMinute() + ":" + (String)myGNSS.getSecond();

  // BNO
  sensors_event_t event;
  bno.getEvent(&event);
  tiltx = (float)event.orientation.x;
  tilty = (float)event.orientation.y;  

  // ADC Voltage
  //voltage = analogRead(A2)*(6.6/8192)*2;
  voltage = analogRead(A2)*(6.6/2048)*2;

  // Mission Time
  int dtime = round(millis()/1000);
  int dhours = int(floor(dtime / 3600));
  dtime = dtime - dhours * 3600;
  int dminutes = int(floor(dtime / 60));
  dtime = dtime - dminutes * 60;
  int dseconds = dtime;
  
  String(startHour + dhours) + ":" + String(startMinute + dminutes) + ":" + String(startSecond + dseconds);
  
}

String packetGenerator(){
  /*
   * Creates a packet to be sent down to the GCS
   * Returns the packet
   */
   packetCount++;
   updateData();
   
   String packet = "1070," 
                  + missionTime + "," 
                  + (String)packetCount + "," 
                  + flightMode + "," 
                  + flightState + "," 
                  + (String)altitude + "," 
                  + hs_deployed + "," 
                  + pc_deployed + "," 
                  + mast_raised + "," 
                  + (String)temperature + "," 
                  + (String)pressure + "," 
                  + (String)voltage + "," 
                  + gps_time + "," 
                  + (String)gps_altitude + "," 
                  + (String)latitude + "," 
                  + (String)longitude + "," 
                  + (String)siv + "," 
                  + (String)tiltx + "," 
                  + (String)tilty + "," 
                  + cmdecho;
    Serial.println(packet);
    return packet;         
}




///////////////////////////////////// STATE LOOP /////////////////////////////////////
void loop() {
packet_csv = SD.open("/testlaunchdata.csv", FILE_APPEND);
  
   if (flightState == "IDLE"){
    ;
  }
  else {
    String packet = packetGenerator();
    writeToFile(packet, packet_csv);
    debugPrintData();
    
    if (flightState == "READY"){
      stateReady();
    }
    else if (flightState == "ASCENDING"){
      stateAscending();
    }
    else if (flightState == "DESCENDING"){
      stateDescending();
    }
    else if (flightState == "HSDEPLOYED"){
      stateShieldRelease();
    }
    else if (flightState == "PCDEPLOYED"){
      stateChuteRelease();
    }
    else if (flightState == "LANDED"){
      stateLanded();
    }
    else {
      Serial.print("Case error.");
    }
  }

  readcommands();
  
  packet_csv.close();

  ledBlink(); // make sure this stays at the end of the loop
}
