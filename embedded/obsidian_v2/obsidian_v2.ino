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
String flightState = "IDLE"; // READY IF TESTING ON ITS OWN, IDLE IF TESTING XBEES
char hs_deployed = 'N';
char pc_deployed = 'N';
char mast_raised = 'N';
String cmdecho = "NONE";
int startHour;
int startMinute;
int startSecond;
String cmd;
String cmdarg;
bool simEnable = false;
bool simActive = false;

// Internal for Reset
bool bmpWorking = true;
bool bnoWorking = true;
bool samWorking = true;
bool sdWorking = true;
int panelPosition = 0;

// Pins
const int ledPin = 12;
const int rServoPin = 32;
const int fServoPin = 15;
const int pServoPin = 14;
const int mosfetPin = 27;
const int buzPin = 21;
const int cameraPin = 4;

File packet_csv;
File backup_txt;
Servo releaseServo;
Servo flagServo;
Servo panelServo;


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

      alt_offset = itemAt(readFile,0).toFloat();
      packetCount = itemAt(readFile,1).toInt();
      flightMode = itemAt(readFile,2).charAt(0); 
      flightState = itemAt(readFile,3);
      hs_deployed = itemAt(readFile,4).charAt(0);
      pc_deployed = itemAt(readFile,5).charAt(0);
      mast_raised = itemAt(readFile,6).charAt(0);
      simEnable = itemAt(readFile,7).toInt() == 1;
      simActive = itemAt(readFile,8).toInt() == 1;
      cmdecho = itemAt(readFile, 9);
      panelPosition = itemAt(readFile, 10).toInt();

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
                           String(simEnable) + "," +
                           String(simActive) + "," +
                           String(cmdecho) + "," +
                           String(panelPosition) + "\n";
      writeToFile(resetPacket, backup_txt); 
      backup_txt.close();
    }
    packet_csv = SD.open("/testlaunchdata.csv", FILE_APPEND);
    if (!packet_csv){
      Serial.println("SD CSV DID NOT OPEN PROPERLY");
    } else {
      writeToFile("TEAM_ID,MISSION_TIME,PACKET_COUNT,MODE,STATE,ALTITUDE,HS_DEPLOYED,PC_DEPLOYED,MAST_RAISED,TEMPERATURE,PRESSURE,VOLTAGE,GPS_TIME,GPS_ALTITUDE,GPS_LATITUDE,GPS_LONGITUDE,GPS_SATS,TILT_X,TILT_Y,CMD_ECHO\n", packet_csv);
    }
    packet_csv.close();
  }
  
  // Servo set up
  releaseServo.attach(rServoPin);
  panelServo.attach(pServoPin);
  flagServo.attach(fServoPin);

  // MOSFET set up
  pinMode(mosfetPin, OUTPUT);

  // Camera set up
  pinMode(cameraPin, OUTPUT);

  // LED set up
  pinMode(ledPin,OUTPUT);

  // Buzzer set up 
  pinMode(buzPin, OUTPUT);

  resetRelease();
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
void raiseFlag() {
  Serial.println("Raising the flag...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  flagServo.write(180);
  digitalWrite(mosfetPin, LOW);
}

void lowerFlag() {
  Serial.println("Lowering the flag...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  flagServo.write(0);
  digitalWrite(mosfetPin, LOW);
}

void releaseContainer() {
  Serial.println("Releasing from the container...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  releaseServo.write(90);
  digitalWrite(mosfetPin, LOW);
}

void releaseParachute() {
  Serial.println("Releasing the parachute...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  releaseServo.write(180);
  digitalWrite(mosfetPin, LOW);
}

void resetRelease() {
  Serial.println("Resetting the release mechanism...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  releaseServo.write(0);
  digitalWrite(mosfetPin, LOW);
}

// 130 closes, 50 opens, 93 stops
void openPanels() {
  Serial.println("Opening the heat shield...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  switch (panelPosition) {
    case 0:
    panelServo.write(130);
    delay(4000);
    panelServo.write(93);
    break;
    
    case 2:
    panelServo.write(50);
    delay(6000);
    panelServo.write(93);
    break;

    case 1:
    default:
    Serial.println("The heat shield is already in position 1.");
    break;
  }
  panelPosition = 1;
  Serial.println("The heat shield is open.");
  digitalWrite(mosfetPin, LOW);
}

void closePanels() {
  Serial.println("Closing the heat shield...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  switch (panelPosition) {
    case 1:
    panelServo.write(50);
    delay(3000);
    panelServo.write(93);
    break;

    case 2:
    panelServo.write(50);
    delay(14000);
    panelServo.write(93);
    break;

    case 0:
    default:
    Serial.println("The panels are already closed!");
    break;
  }
  panelPosition = 0;
  Serial.println("The heat shield is closed.");
  digitalWrite(mosfetPin, LOW);
}

void upright() {
  Serial.println("Opening the heat shield...");
  digitalWrite(mosfetPin, HIGH);
  delay(500);
  switch (panelPosition) {
    case 0:
    panelServo.write(130);
    delay(6000);
    panelServo.write(93);
    break;

    case 1:
    panelServo.write(130);
    delay(14000);
    panelServo.write(93);
    break;

    case 2:
    default:
    Serial.println("The panels have already uprighted!");
    break;
  }
  panelPosition = 2;
  Serial.println("The heat shield is open.");
  digitalWrite(mosfetPin, LOW);
}

void resetMechanisms() {
  Serial.println("Resetting the mechanisms...");
  closePanels();

  resetRelease();

  lowerFlag();
  Serial.println("All mechanisms are reset.");
}

void startRecording() {
  Serial.println("Recording...");
  digitalWrite(cameraPin, LOW); 
  delay(1000);
  digitalWrite(cameraPin, HIGH);
  Serial.println("Recording started.");
}

void stopRecording() {
  Serial.println("Stopping the recording...");
  digitalWrite(cameraPin, LOW);
  delay(1000);
  digitalWrite(cameraPin, HIGH);
  delay(10);
  digitalWrite(cameraPin, LOW);
  Serial.println("Recording stopped.");
}

// led setup 
void ledBlink(){
  pinMode(ledPin,HIGH);
  delay(100);
  pinMode(ledPin,LOW);
}

// buzzer setup
void buzzer(){
  pinMode(buzPin,HIGH);
  delay(100);
  pinMode(buzPin,LOW);
}


///////////////////////////////////// COMMAND READING /////////////////////////////////////
void readcommands(String cmd, String cmdarg){
  /*if (Serial1.available()){
    String packet = Serial1.readString();
    if (itemAt(packet, 0) == "CMD" && itemAt(packet, 1) == "1070"){
      cmd = itemAt(packet, 2);
      cmdarg = itemAt(packet, 3);
    */
  if (cmd == "CX"){
    if (cmdarg == "ON\n"){
      Serial.println("CXON");
      cmdecho = "CXON";
      flightState = "READY";
    } else if (cmdarg == "OFF\n"){
      Serial.println("CXOFF");
      cmdecho = "CXOFF";
      flightState = "IDLE";
    } else {
      Serial.println("Invalid command received.");
    }

  } else if (cmd == "ST"){
    if (cmdarg == "GPS\n") {
      Serial.println("STGPS");
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
      Serial.println("STCUS");
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
    if (cmdarg == "ENABLE\n") {
      Serial.println("SIME");
      cmdecho = "SIME";
      simEnable = true;
      flightMode = 'S';
      
    } else if (cmdarg == "DISABLE\n") {
      Serial.println("SIMD");
      cmdecho = "SIMD";
      simActive = false;
      simEnable = false;
      flightMode = 'F';
      flightState = "IDLE";
      
    } else if (cmdarg == "ACTIVATE\n") {
      Serial.println("SIMA");
      cmdecho = "SIMA";
      simActive = true;
      flightState = "READY";
   
    } else {
      Serial.println("Invalid command received.");
    }
  } 
  else if (cmd == "CAL\n"){
    Serial.println("CAL");
    cmdecho = "CAL";
    
    float number1 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    float number2 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    float number3 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    float number4 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    float number5 = bmp.readAltitude(SEALEVELPRESSURE_HPA);;
    
    alt_offset = (number1 + number2 + number3 + number4 + number5)/5;

  } else if (cmd == "ACT"){
    if (cmdarg == "MR\n") {
      Serial.println("ACTMR");
      cmdecho = "ACTMR";
      releaseContainer();
    } else if (cmdarg == "HS\n") {
      Serial.println("ACTHS");
      cmdecho = "ACTHS";
      upright();
    } else if (cmdarg == "PC\n") {
      Serial.println("ACTPC");
      cmdecho = "ACTPC";
      pc_deployed = 'C';
      releaseParachute();
    } else if (cmdarg == "AB\n") {
      Serial.println("ACTAB");
      cmdecho = "ACTAB";
      buzzer();
    } else if (cmdarg == "LED\n") {
      Serial.println("ACTLED");
      cmdecho = "ACTLED";
      ledBlink();
    } else {
      Serial.println("Invalid command received.");
    }

  } else if (cmd == "RESREL\n") {
    Serial.println("RESREL");
    cmdecho = "RESREL";
    resetMechanisms();

  } else {
    Serial.println("Invalid command received.");
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
    if (!(simActive && simEnable))
    {  
      pressure = round(bmp.pressure / 100.0)/10.0;
      last_alt = altitude;
      altitude = round(10 * (bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset))/10.0;
    }
    temperature = round(10 * bmp.temperature)/10.0;
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
    tiltx = 90 - (float)event.orientation.y;
    tilty = 180 - (float)event.orientation.z;  
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
                         String(simEnable) + "," +
                         String(simActive) + "," +
                         String(cmdecho) + "," + 
                         String(panelPosition) + '\n';
                         
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
    else if (flightState == "ASCENDING" && altitude > 200 && altitude - last_alt < 0){
      startRecording();
      flightState = "DESCENDING";
    }
    else if (flightState == "DESCENDING" && altitude <= 500){
      releaseContainer();
      openPanels();
      flightState = "HSDEPLOYED";
      hs_deployed = 'P';

    }
    else if (flightState == "HSDEPLOYED" && altitude <= 200){
      closePanels();
      releaseParachute();
      flightState = "PCDEPLOYED";
      pc_deployed = 'C';
    }
    else if (flightState == "PCDEPLOYED" && altitude - last_alt <= 1 && altitude - last_alt >= -1){
      upright();
      stopRecording();
      raiseFlag();
      flightState = "LANDED";
      mast_raised = 'M';
    }
    else if (flightState == "LANDED"){
      buzzer();
    }
    else {
      Serial.println("No action required for the cases.");
    }
  }
  
  if (sdWorking){
    packet_csv.close();
  }

  if (!(simActive and simEnable) and Serial1.available()) {
    String packet = Serial1.readString();
    Serial.print(packet);
  
    if (itemAt(packet, 0) == "CMD" and itemAt(packet, 1) == "1070") {
      readcommands(itemAt(packet, 2), itemAt(packet, 3)); 
      // maybe check if item 3 is CMD?? that would happen if there is no arg like in CAL
    }
  } 
  else if (simActive and simEnable){
    while (true){
      while (!Serial1.available()){;}
      String packet = Serial1.readString();
      Serial.print(packet);
    
      if (itemAt(packet,0) == "CMD" && itemAt(packet, 1) == "1070"){
        cmd = itemAt(packet, 2);
        cmdarg = itemAt(packet, 3);
      
        if (cmd == "SIMP"){
          pressure = cmdarg.toFloat()/100; // convert string to float
          altitude = round(10 * (bmp.readAltitude(pressure)))/10.0;
          cmdecho = "SIMP";
          Serial.println("SIMP");
          break;
        }
        else {
          readcommands(cmd, cmdarg);
        }
      }
    }
  }

  ledBlink(); // make sure this stays at the end of the loop
}
