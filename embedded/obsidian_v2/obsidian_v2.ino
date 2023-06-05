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
TaskHandle_t task1;

// BMP
float temperature;
float altitude;
float pressure;
float alt_offset;
double last_alt;
bool offset_set;

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

// Buzzer
bool buzEnable = false;

// HS Start Condition
bool hsCondition = false;
bool hsDeployed = false;

File packet_csv;
File backup_txt;
Servo releaseServo;
Servo flagServo;
Servo panelServo;

///////////////////////////////////// SETUP /////////////////////////////////////
void setup() {  
  Serial.begin(9600);
  Serial1.begin(115200); // xbee communication
  Wire.begin();
  xTaskCreatePinnedToCore(
            preciseTimedFuncs,  /* Task function. */
            "timedFuncs",       /* name of task. */
            10000,              /* Stack size of task */
            NULL,               /* parameter of the task */
            tskIDLE_PRIORITY,   /* priority of the task */
            &task1,             /* Task handle to keep track of created task */
            0);                 /* pin task to core 0 */

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
    offset_set = true;
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

///////////////////////////////////// MECHANISMS /////////////////////////////////////
void setFlagPosition(int pos) {
  // 0 lowers the flag, 1 raises the flag
  digitalWrite(mosfetPin, HIGH);
  delay(250);

  if (pos == 0 || pos == 1) {
    flagServo.write(pos * 180);
    if (pos) {
      mast_raised = 'M';
    } else {
      mast_raised = 'N';
    }
  } else {
    Serial.println("Invalid flag position.");
  }

  digitalWrite(mosfetPin, LOW);
}

void setReleasePosition(int pos) {
  // 0 closes the release, 1 releases the container, 2 releases the parachute
  digitalWrite(mosfetPin, HIGH);
  delay(250);

  if (pos == 0 || pos == 1 || pos == 2) {
    releaseServo.write(pos * 90);
    if (pos == 1) {
      hs_deployed = 'P';
    } else if (pos == 2){
      pc_deployed = 'C';
    }
  } else {
    Serial.println("Invalid release position.");
  }

  digitalWrite(mosfetPin, LOW);
}

void setShieldPosition(int pos) {
  if (pos == panelPosition) {
    Serial.println("The heat shield is already at that position.");
    return;
  }

  digitalWrite(mosfetPin, HIGH);
  delay(250);

  if (pos > panelPosition) { // heat shield needs to open
    panelServo.write(130);

    if (panelPosition == 0 && pos == 1) {
      delay(3000);

    } else if (panelPosition == 0 && pos == 2) {
      delay(35000);

    } else if (panelPosition == 1 && pos == 2) {
      delay(12000);

    } else {
      panelServo.write(93);
      Serial.println("Invalid opening case.");
      return;

    }
  } else { // heat shield needs to close
    panelServo.write(50);

    if (panelPosition == 2 && pos == 1) {
      delay(12000);

    } else if (panelPosition == 2 && pos == 0) {
      delay(12000);

    } else if (panelPosition == 1 && pos == 0) {
      delay(4000);

    } else {
      panelServo.write(93);
      Serial.println("Invalid closing case.");
      return;

    }
  }
  panelPosition = pos;
  panelServo.write(93);

  digitalWrite(mosfetPin, LOW);
}

void startRecording() {
  Serial.println("Recording...");
  digitalWrite(cameraPin, LOW); 
  delay(1000);
  digitalWrite(cameraPin, HIGH);
  Serial.println("Recording started.");
  
  delay(1000);
  
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
 
void ledBlink(){
  digitalWrite(ledPin,HIGH);
  delay(100);
  digitalWrite(ledPin,LOW);
}

void buzzer(){
  digitalWrite(buzPin,HIGH);
  delay(500);
  digitalWrite(buzPin,LOW);
}

///////////////////////////////////// COMMAND READING /////////////////////////////////////
void readcommands(String cmd, String cmdarg){
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
      offset_set = false;
      alt_offset = 0;
   
    } else {
      Serial.println("Invalid command received.");
    }
  } else if (cmd == "CAL\n"){
    Serial.println("CAL");
    cmdecho = "CAL";
    alt_offset = setDefaultAlt();

  } else if (cmd == "ACT"){
    if (cmdarg == "AB\n") {
      Serial.println("ACTAB");
      cmdecho = "ACTAB";
      buzzer();

    } else if (cmdarg == "LED\n") {
      Serial.println("ACTLED");
      cmdecho = "ACTLED";
      ledBlink();

    } else if (cmdarg == "FL0\n") {
      Serial.println("ACTFL0");
      cmdecho = "ACTFL0";
      setFlagPosition(0);

    } else if (cmdarg == "FL1\n") {
      Serial.println("ACTFL1");
      cmdecho = "ACTFL1";
      setFlagPosition(1);

    } else if (cmdarg == "RL0\n") {
      Serial.println("ACTRL0");
      cmdecho = "ACTRL0";
      setReleasePosition(0);

    } else if (cmdarg == "RL1\n") {
      Serial.println("ACTRL1");
      cmdecho = "ACTRL1";
      setReleasePosition(1);
      
    } else if (cmdarg == "RL2\n") {
      Serial.println("ACTRL2");
      cmdecho = "ACTRL2";
      setReleasePosition(2);

    } else if (cmdarg == "HS0\n") {
      Serial.println("ACTHS0");
      cmdecho = "ACTHS0";
      setShieldPosition(0);

    } else if (cmdarg == "HS1\n") {
      Serial.println("ACTHS1");
      cmdecho = "ACTHS1";
      setShieldPosition(1);

    } else if (cmdarg == "HS2\n") {
      Serial.println("ACTHS2");
      cmdecho = "ACTHS2";
      setShieldPosition(2);

    } else if (cmdarg == "RES\n") {
      Serial.println("ACTRES");
      cmdecho = "ACTRES";
      setFlagPosition(0);
      setReleasePosition(0);
      setShieldPosition(0);
      hs_deployed = 'N';
      pc_deployed = 'N';
      mast_raised = 'N';

    } else {
      Serial.println("Invalid command received.");
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
    last_alt = altitude;
    if (!(simActive && simEnable)) {  
      pressure = round(bmp.pressure / 100.0)/10.0;
      altitude = round(10 * (bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset))/10.0;
    } else if (pressure != 0) {
      // this formula will kill me
      altitude = (temperature + 273.15)/(-0.0065) * (pow(((pressure*1000)/(SEALEVELPRESSURE_HPA*100)), ((-8.31432*-0.0065)/(9.80665*0.0289644))) - 1) - alt_offset;
      Serial.println("SIMP altitude: " + String(altitude));
      if (!offset_set) {
        alt_offset = altitude;
        Serial.println("SIMP alt_offset: " + String(alt_offset));
        altitude = altitude - alt_offset;
        offset_set = true;
      }
    } else {
      altitude = 0;
    }
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
    tiltx = (float)event.orientation.z;
    tilty = 90 - (float)event.orientation.y;  
  }

  // ADC Voltage
  voltage = analogRead(13)*(3.3/512);

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

// Core 0 // (Buzzer, LED, Servo)
void preciseTimedFuncs(void * parameters) {
  delay(500);

  // Servos are checked while the LED and Buzzer are waiting
  while (true) {
    int startTime = millis();
    int currentTime = millis();
    bool ledFlip = false;
    bool buzFlip = false;
    int ledTime = 1000;
    int buzTime = 5000;
    digitalWrite(ledPin, (ledFlip ? LOW : HIGH));
    ledFlip = !ledFlip;
    if (buzEnable) {
      digitalWrite(buzPin, (buzFlip ? LOW : HIGH));
      buzFlip = !buzFlip;
    }
    
    // Buzzer
    while (currentTime - startTime <= buzTime) {
      if (hsCondition && !(hsDeployed)) {
        break;
      }
      
      // LED
      if (currentTime - startTime > ledTime) {
        digitalWrite(ledPin, (ledFlip ? LOW : HIGH));

        ledTime += 1000;
        ledFlip = !ledFlip;
      }
      // Serial.println(currentTime - startTime);
      currentTime = millis();
    }
    digitalWrite(ledPin, LOW);
    ledFlip = true;
    if (buzEnable) {
      digitalWrite(buzPin, (buzFlip ? LOW : HIGH));
      buzFlip = !buzFlip;
    }
    
    buzTime += buzTime;

    // Buzzer
    while (currentTime - startTime <= buzTime) {
      if (hsCondition && !(hsDeployed)) {
        break;
      }
      
      // LED
      if (currentTime - startTime > ledTime) {
        digitalWrite(ledPin, (ledFlip ? LOW : HIGH));
        ledTime += 1000;
        ledFlip = !ledFlip;
      }
      // Serial.println(currentTime - startTime);
      currentTime = millis();
    }
    digitalWrite(ledPin, HIGH);
    ledFlip = false;
    if (buzEnable) {
      digitalWrite(buzPin, (buzFlip ? LOW : HIGH));
      buzFlip = !buzFlip;
    }

    // LED and Buzzer are managed while waiting for HS to deploy
    if (hsCondition && !(hsDeployed) == true) {
      startTime = millis();
      currentTime = millis();
      // start servo
      setShieldPosition(2);
      hsDeployed = true;
    }
  }
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
    
    if (flightState == "READY" && altitude >= 50){
      flightState = "ASCENDING";

    } else if (flightState == "ASCENDING" && altitude > 200 && altitude - last_alt < 0){
      startRecording();
      flightState = "DESCENDING";

    } else if (flightState == "DESCENDING" && altitude <= 500){
      setReleasePosition(1);
      setShieldPosition(1);
      flightState = "HSDEPLOYED";
      hs_deployed = 'P';

    } else if (flightState == "HSDEPLOYED" && altitude <= 200){
      setReleasePosition(2);
      setShieldPosition(0);
      flightState = "PCDEPLOYED";
      pc_deployed = 'C';

    } else if (flightState == "PCDEPLOYED" && altitude - last_alt <= 1 && altitude - last_alt >= -1){
      hsCondition = true;
      setFlagPosition(1);
      stopRecording();

      flightState = "LANDED";
      mast_raised = 'M';
    } else if (flightState == "LANDED"){
      buzEnable = true;
    } else {
      Serial.println("No action required for the cases.");
    }
  }
  
  if (sdWorking){
    packet_csv.close();
  }

  if (!(simActive and simEnable) and Serial1.available()) {
      String packet = Serial1.readStringUntil('\n');
      packet = packet + "\n";
      Serial.print(packet);
  
    if (itemAt(packet, 0) == "CMD" and itemAt(packet, 1) == "1070") {
      readcommands(itemAt(packet, 2), itemAt(packet, 3)); 
      // maybe check if item 3 is CMD?? that would happen if there is no arg like in CAL
    }
  } 
  else if (simActive and simEnable){
    while (true){
      while (!Serial1.available()){;}
//        String packet = Serial1.readString();
//        Serial.print(packet);
    
        String packet = Serial1.readStringUntil('\n');
        packet = packet + "\n";
        Serial.print(packet);
    
      if (itemAt(packet,0) == "CMD" && itemAt(packet, 1) == "1070"){
        cmd = itemAt(packet, 2);
        cmdarg = itemAt(packet, 3);
      
        if (cmd == "SIMP"){
          pressure = cmdarg.toFloat()/1000; // convert string to float
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
  // for john <3
}
