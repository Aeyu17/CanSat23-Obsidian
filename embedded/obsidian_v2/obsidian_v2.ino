#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SD.h>
#include <ESP32Servo.h>

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
String flightState = "READY"; // READY IF TESTING ON ITS OWN, IDLE IF TESTING XBEES
char hs_deployed = 'N';
char pc_deployed = 'N';
char mast_raised = 'N';
String cmdecho = "NONE";

// Internal for Reset
int startHour;
int startMinute;
int startSecond;
bool containerReleased;
bool shieldDeployed;
bool chuteReleased;
bool flagRaised;
bool bmpWorking = true;
bool bnoWorking = true;
bool samWorking = true;
bool sdWorking = true;

int ledPin = 27;
int rServoPin = 32;
int fServoPin = 15;
int hServoPin = 14;
int buzPin = 21;

File packet_csv;
File backup_txt;
Servo release_servo;
Servo flag_servo;
Servo hs_servo;


///////////////////////////////////// SETUP /////////////////////////////////////
void setup() {  
  Serial.begin(115200);
  Serial1.begin(9600); // xbee communication
  Wire.begin();

  // BMP set up
  if (!bmp.begin_I2C()) {
    Serial.println("BMP NOT WORKING");
    bmpWorking = false;
    
    temperature = -999;
    altitude = -999;
    pressure = -999;
    alt_offset = -999;
    last_alt = -999;
  } else {
    Serial.println("BMP WORKING");
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  
    // get rid of bad altitude data
    bmp.readAltitude(SEALEVELPRESSURE_HPA);
    bmp.readAltitude(SEALEVELPRESSURE_HPA);
    bmp.readAltitude(SEALEVELPRESSURE_HPA);
  
    alt_offset = setDefaultAlt();
  }

  // SAM set up
  if (!myGNSS.begin()){
    Serial.println("SAM NOT WORKING");

    samWorking = false;
    startHour = 0;
    startMinute = 0;
    startSecond = 0;
    latitude = -999;
    longitude = -999;
    gps_altitude = -999;
    siv = -999;
    gps_time = "99:99:99";
    gps_hour = "99";
    gps_min = "99";
    gps_sec = "99";
  } else {
    Serial.println("SAM WORKING");
    myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
    myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
    
    startHour = myGNSS.getHour();
    startMinute = myGNSS.getMinute();
    startSecond = myGNSS.getSecond();

    // myGNSS.enableDebugging(); // in case of error
  }
  
  // BNO set up
  if (!bno.begin()){
    Serial.println("BNO NOT WORKING");

    bnoWorking = false;
    tiltx = -999;
    tilty = -999;
  } else {
    Serial.println("BNO WORKING");
    bno.setExtCrystalUse(true);
  }

  // SD set up
  if (!SD.begin()){
    Serial.println("SD NOT WORKING");

    sdWorking = false;
  } else {
    Serial.println("SD WORKING");
    backup_txt = SD.open("/reset.txt", FILE_READ);

    if (backup_txt) {
      // RESTORE TXT VALUES HERE
      backup_txt.seek(0);
      String readFile = backup_txt.readStringUntil('\n');

      alt_offset = itemAt(readFile,0).toInt();
      packetCount = itemAt(readFile,1).toInt();
      flightMode = itemAt(readFile,2).charAt(0); 
      flightState = itemAt(readFile,3);
      hs_deployed = itemAt(readFile,4).charAt(0);
      pc_deployed = itemAt(readFile,5).charAt(0);
      mast_raised = itemAt(readFile,6).charAt(0);
      containerReleased = (itemAt(readFile, 7).equals("1"));
      shieldDeployed = (itemAt(readFile, 8).equals("1"));
      chuteReleased = (itemAt(readFile, 9).equals("1"));
      flagRaised = (itemAt(readFile, 10).equals("1"));
      cmdecho = itemAt(readFile,11);

    } else {
      // SET UP BACKUP TXT
      backup_txt.close(); // do we need this?
      backup_txt = SD.open("/reset.txt", FILE_WRITE);
      String resetPacket = String(alt_offset) + "," + 
                           String(packetCount) + "," + 
                           String(flightMode) + "," + 
                           String(flightState) + "," + 
                           String(hs_deployed) + "," + 
                           String(pc_deployed) + "," + 
                           String(mast_raised) + "," + 
                           String(containerReleased) + "," +
                           String(shieldDeployed) + "," +
                           String(chuteReleased) + "," + 
                           String(flagRaised) + "," +
                           String(cmdecho) + "\n";
      writeToFile(resetPacket, backup_txt); 
      backup_txt.close();
    }
    packet_csv = SD.open("/testlaunchdata.csv", FILE_APPEND);
    if (!packet_csv){
      Serial.println("SD CSV DID NOT OPEN PROPERLY");
    } else {
      writeToFile("TEAM_ID,MISSION_TIME,PACKET_COUNT,MODE,STATE,ALTITUDE,HS_DEPLOYED,PC_DEPLOYED,MAST_RAISED,TEMPERATURE,VOLTAGE,PRESSURE,GPS_TIME,GPS_ALTITUDE,GPS_LATITUDE,GPS_LONGITUDE,GPS_SATS,TILT_X,TILT_Y,CMD_ECHO", packet_csv);
    }
    packet_csv.close();
  }
  
  // Servo set up
  release_servo.attach(rServoPin); // may need to change
  hs_servo.attach(hServoPin); // need to change
  flag_servo.attach(fServoPin); // need to change

  // MOSFET set up
  pinMode(A5, INPUT);

  // Camera set up
  // TODO
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
  pinMode(ledPin,OUTPUT);

  pinMode(ledPin,HIGH);
  delay(100);
  pinMode(ledPin,LOW);
}

// buzzer setup
void buzzer(){
  pinMode(buzPin, OUTPUT);

  pinMode(buzPin,HIGH);
  delay(100);
  pinMode(buzPin,LOW);
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
void containerRelease() {
  if (containerReleased){
    return;
  }
  pinMode(A5, HIGH);
  containerReleased = true;
  release_servo.write(90);
  pinMode(A5, LOW);
}


// first shield deploy
void shieldDeploy() {
  if (shieldDeployed){
    return;
  }
  pinMode(A5, HIGH);
  shieldDeployed = true;
  hs_deployed = 'P';
  flightState = "HSDEPLOYED";
  hs_servo.write(2400); // don't know this number yet
  pinMode(A5, LOW);
}

void shieldRetract() {
  if (!shieldDeployed){
    return;
  }
  pinMode(A5, HIGH);
  shieldDeployed = false;
  hs_servo.write(1700); // fuck???
  pinMode(A5, LOW);
}

// parachute release
void chuteRelease() {
  if (chuteReleased){
    return;
  }
  pinMode(A5, HIGH);
  chuteReleased = true;
  pc_deployed = 'C';
  flightState = "PCDEPLOYED";
  release_servo.write(0);
  pinMode(A5, LOW);
}


// second shield deploy
void upright() {
  pinMode(A5, HIGH);
  hs_servo.write(180); // don't know this number yet
  flightState = "LANDED";
  shieldDeployed = true;
  pinMode(A5, LOW);
}

// flag delpy
void flagDeploy() {
  pinMode(A5, HIGH);
  mast_raised = 'M';
  flagRaised = true;
  flag_servo.write(180); // don't know this number yet
  pinMode(A5, LOW);
}




///////////////////////////////////// COMMAND READING /////////////////////////////////////
void readcommands(){
  if (Serial1.available()){
    String packet = Serial1.readString();
    if (itemAt(packet, 0) == "CMD" && itemAt(packet, 1) == "1070"){
      String cmd = itemAt(packet, 2);
      String cmdarg = itemAt(packet, 3);
      if (cmd == "CX"){
        if (cmdarg == "ON"){
          cmdecho = "CXON";
          flightState = "READY";
        } else if (cmdarg == "OFF"){
          cmdecho = "CXOFF";
          flightState = "IDLE";
        } else {
          Serial.println("Invalid command received.");
        }
      } else if (cmd == "ST"){
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

          while (startSecond < 0){
            startSecond += 60;
            startMinute--;
          }
          while (startMinute < 0){
            startMinute += 60;
            startHour--;
          }
          while (startHour < 0){
            startHour += 24;
          }          
        } else {
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
      } else if (cmd == "SIM"){
        if (cmdarg = "ENABLE") {
          cmdecho = "SIME";
          
        } else if (cmdarg = "DISABLE") {
          cmdecho = "SIMD";
          
        } else if (cmdarg = "ACTIVATE") {
          cmdecho = "SIMA";
          
        } else {
          Serial.println("Invalid command received.");
        }
      } else if (cmd == "SIMP"){
        cmdecho = "SIMP";
        // TODO

      } else if (cmd == "CAL"){
        cmdecho = "CAL";
        
        float number1 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number2 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number3 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number4 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
        float number5 = bmp.readAltitude(SEALEVELPRESSURE_HPA);;
        
        alt_offset = (number1 + number2 + number3 + number4 + number5)/5;
      } else if (cmd == "ACT"){
        if (cmdarg == "MR") {
          cmdecho = "ACTMR";
          containerReleased = false;
          containerRelease();
        } else if (cmdarg == "HS") {
          cmdecho = "ACTHS";
          shieldDeploy();
        } else if (cmdarg == "PC") {
          cmdecho = "ACTPC";
          chuteRelease();
        } else if (cmdarg == "AB") {
          cmdecho = "ACTAB";
          buzzer();
        } else if (cmdarg == "LED") {
          cmdecho = "ACTLED";
          ledBlink();
        } else {
          Serial.println("Invalid command received.");
        }
      } else if (cmd == "RESREL") {
        cmdecho = "RESREL";
        release_servo.write(180);
        flag_servo.write(0);
        // adam give us the degrees thanks
        // hs_servo.write();

      } else {
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
  if (bmpWorking) {
    temperature = round(10 * bmp.temperature)/10.0;
    pressure = round(bmp.pressure / 100.0)/10.0;
    last_alt = altitude;
    altitude = round(10 * (bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset))/10.0;
  }

  // SAM
  if (samWorking) {
    latitude = myGNSS.getLatitude()/10000000.0;
    longitude = myGNSS.getLongitude()/10000000.0;
    gps_altitude = myGNSS.getAltitude()/1000.0;
    siv = myGNSS.getSIV();
    gps_time = (String)myGNSS.getHour() + ":" + (String)myGNSS.getMinute() + ":" + (String)myGNSS.getSecond();
  }
  
  // BNO
  if (bnoWorking) {
    sensors_event_t event;
    bno.getEvent(&event);
    tiltx = -(float)event.orientation.y;
    tilty = -(float)event.orientation.z;  
  }

  // ADC Voltage
  voltage = analogRead(39)*(3.3/16384);

  // Mission Time
  int dtime = round(millis()/1000);
  int dhours = int(floor(dtime / 3600));
  dtime = dtime - dhours * 3600;
  int dminutes = int(floor(dtime / 60));
  dtime = dtime - dminutes * 60;
  int dseconds = dtime;

  while ((startSecond + dseconds) >= 60){
    dseconds -= 60;
    dminutes++;
  }
  while ((startMinute + dminutes) >= 60){
    dminutes -= 60;
    dhours++;
  }
  while ((startHour + dhours) >= 24){
    dhours -= 24;
  }

  String hourStr = String(startHour + dhours);
  String minStr = String(startMinute + dminutes);
  String secStr = String(startSecond + dseconds);

  if (hourStr.length() == 1){
    hourStr = "0" + hourStr;
  }
  if (minStr.length() == 1){
    minStr = "0" + minStr;
  }
  if (secStr.length() == 1){
    secStr = "0" + secStr;
  }
  
  missionTime = hourStr + ":" + minStr + ":" + secStr;
  
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
                  + cmdecho + '\n';
    Serial.println(packet);
    return packet;         
}




///////////////////////////////////// FLIGHT STATE LOOP /////////////////////////////////////
void loop() {
 
  if (sdWorking){
    packet_csv = SD.open("/testlaunchdata.csv", FILE_APPEND);
    backup_txt = SD.open("/reset.txt", FILE_WRITE);

    String resetPacket = String(alt_offset) + "," + 
                         String(packetCount) + "," + 
                         String(flightMode) + "," + 
                         String(flightState) + "," + 
                         String(hs_deployed) + "," + 
                         String(pc_deployed) + "," + 
                         String(mast_raised) + "," + 
                         String(containerReleased) + "," +
                         String(shieldDeployed) + "," +
                         String(chuteReleased) + "," + 
                         String(flagRaised) + "," +
                         String(cmdecho) + '\n';
                         
    writeToFile(resetPacket, backup_txt); 
    backup_txt.close();
  }
  
  if (flightState != "IDLE") {
    String packet = packetGenerator();
    if (sdWorking){
      writeToFile(packet, packet_csv);
    }
    Serial1.print(packet);
    debugPrintData();
    
    if (flightState == "READY" && altitude >= 5){
      flightState = "ASCENDING";
    }
    else if (flightState == "ASCENDING" && altitude - last_alt < 0){
      flightState = "DESCENDING";
    }
    else if (flightState == "DESCENDING" && altitude <= 500){
      containerRelease();
      shieldDeploy();
    }
    else if (flightState == "HSDEPLOYED" && shieldDeployed && altitude <= 200){
      shieldRetract();
      chuteRelease();
    }
    else if (flightState == "PCDEPLOYED" && altitude - last_alt <= 1 && altitude - last_alt >= -1 && chuteReleased){
      upright();
      delay(3000);
      flagDeploy();
    }
    else if (flightState == "LANDED"){
      buzzer();
    }
    else {
      Serial.println("No action required for the cases.");
    }
  }

  readcommands();
  
  if (sdWorking){
    packet_csv.close();
  }

  buzzer();
  ledBlink(); // make sure this stays at the end of the loop
}
