#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_config_keys.h>
#include <u-blox_structs.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Servo.h>
#include <string.h>
#include <SoftwareSerial.h>

// TO DO LIST
// testing
// sd reader/writer
  // find libraries for new one and write code
// gpstime
  // NEED TO HANDLE RESET
// cmdecho
  // NEED TO ADD HANDLER FOR WRONG LINE
  // simulation mode
// set decimal places
  // use round() function. Need to wait and see data
  // need to test and see what they are first

///////////////////////////////////// INITIALIZING /////////////////////////////////////
char shield = 'N';
char flag = 'N';
char parachute = 'N';
char modes = 'S';
String state = "IDLE";
const int ID = 1070;
float GROUND = 0;
int packets = 0;
double alt_offset;
double altitude;
double pressure;
bool Rocket = false;
bool ShieldOne = false;
bool ShieldTwoChute = false;
bool ShieldThree = false;
bool Flag = false;
int hours;
int mins;
int secs;
int gpshour;
int gpsmin; 
int gpssec;
String cmdecho;
double lastaltitude;

Servo servo1; // rocket and parachute
Servo servo2; // heat shield
Servo servo3; // flag

SFE_UBLOX_GNSS myGNSS;
long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.

Adafruit_BMP3XX bmp;

// define sea level pressure (will probably need to change)
#define SEALEVELPRESSURE_HPA (1017)

Adafruit_BNO055 bno = Adafruit_BNO055(55);


///////////////////////////////////// SENSOR SETUP /////////////////////////////////////

// BMP388
// BNO055
// SAM-M8Q
// OpenLog // NEED TO DO
// Led

// bmp388 setup
void bmpsetup() {
  
  if (!bmp.begin_I2C()) {   // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
  }
  
  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
}


// bno055 setup
void bnosetup() {
  /* Initialise the sensor */
  if(!bno.begin()){
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
  }
  delay(1000);
    
  bno.setExtCrystalUse(true);
}


// gps setup 
void samsetup() {

  if (!myGNSS.begin()) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  gpshour = myGNSS.getHour();
  gpsmin = myGNSS.getMinute();
  gpssec = myGNSS.getSecond();
}


// led setup 
void ledblink(){
  pinMode(27,OUTPUT);

  pinMode(27,HIGH);
  delay(500);
  pinMode(27,LOW);
  delay(500);
}

// buzzer setup
void buzzer(){
  pinMode(30, OUTPUT); // figure it out dumbass

  pinMode(30,HIGH);
  delay(500);
  pinMode(30,LOW);
  delay(500);
}

// configuring the pressure to ground 
void bmpconfigure() {
 
  double number1;
  double number2;
  double number3;
  double number4;
  double number5;
 
  bmp.readAltitude(SEALEVELPRESSURE_HPA);
  delay(3000);
  
  number1 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  number2 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  number3 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  number4 = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  number5 = bmp.readAltitude(SEALEVELPRESSURE_HPA);;
  
  alt_offset = (number1 + number2 + number3 + number4 + number5)/5;
  Serial.print(alt_offset);
}


///////////////////////////////////// RELEASE MECHANISMS /////////////////////////////////////
// initial rocket release
void rocketrelease() {
  servo1.write(40);
  delay(300);
}

// parachute release
void chuterelease() {
  servo1.write(40);
  delay(300);
}

// first shield deploy
void shielddeploy1() {
  servo2.write(10000); // don't know this number yet
  delay(300);
}

// second shield deploy
void upright() {
  servo2.write(60); // don't know this number yet
  delay(300);
}

// flag delpy
void flagdeploy() {
  servo3.write(500); // don't know this number yet
  delay(300);
}


///////////////////////////////////// READING DATA /////////////////////////////////////
void data() {
  
  // bmp388 reading
  if (!bmp.performReading()) {
    Serial.println("Failed to perform reading D:");
  }

  // for transmit
  bmp.performReading();


  // bno055 reading
  sensors_event_t event; 
  bno.getEvent(&event);

  /* Display the floating point data */
  double roll = event.orientation.x;
  double pitch = event.orientation.y;
  double yaw = event.orientation.z;
 

  // sam-m8q reading
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  long gpslat;
  long gpslong;
  int gpssat;
  long gpsalt;
  
  gpslat = myGNSS.getLatitude();
  gpslong = myGNSS.getLongitude();
  gpsalt = myGNSS.getAltitude();
  gpssat = myGNSS.getSIV();

  String missionTime = iterateTime();

  // gpstime
  gpshour = myGNSS.getHour();
  gpsmin = myGNSS.getMinute();
  gpssec = myGNSS.getSecond();
  String gpstime = String(gpshour) + ":" + String(gpsmin) + ":" + String(gpssec);
  
  
  // voltage                                         
  int vread = analogRead(13);
  float voltage = vread * (1.6/1023.0) * 2;

  // altitude and temperature to display
  double temperature = bmp.temperature;
  lastaltitude = altitude;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset;
  pressure = bmp.pressure/1000.0;
  packets += 1;
  String c = ",";

// TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, 
// PC_DEPLOYED, MAST_RAISED, PRESSURE, TEMPERATURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, 
// GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO


  Serial.println(String(ID) + c + String(missionTime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(pressure) + c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslat) + c + String(gpslong) + c + String(gpssat) + 
  c + String(roll) + c + String(pitch) + c + String(cmdecho));

  
  // XBee
  Serial1.println(String(ID) + c + String(missionTime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(pressure) + c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslat) + c + String(gpslong) + c + String(gpssat) + 
  c + String(roll) + c + String(pitch) + c + String(cmdecho));

  delay(1000);
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

String iterateTime(){
  // time is formatted as hh:mm:ss
  int dtime = round(millis()/1000);
  
  int dhours = int(floor(dtime / 3600));
  dtime = dtime - dhours * 3600;
  int dminutes = int(floor(dtime / 60));
  dtime = dtime - dminutes * 60;
  int dseconds = dtime;

  return String(hours + dhours) + ":" + String(mins + dminutes) + ":" + String(secs + dseconds);
}

float roundTo(float num, int k) {
     num is the number to round, k is the number of decimal places you want
     return round(num * pow(10, k)) / pow(10, k);
     }


///////////////////////////////////// FLIGHT STAGES /////////////////////////////////////
void readie() {
  if (altitude <= 1){
    state = "ASCENDING";
  }
  data();
}

 
// ascending (0m to 700m)
void ascending(){
  if (altitude <= 700 ) {
    data();
  }
  else {
    state = "DESCENDING";
  }
}


// descending (700m to 500m)
void descending() {
  if (!Rocket) {
    rocketrelease();
    Rocket = true;
  }
  data();
  if (altitude <= 500) {
    state = "HSDEPLOYED";
  }
}


// shield_release (500m to 200m)
void shield_release() {
  if (shield == 'N') {
    shielddeploy1();
    shield = 'P';
  }
  data();
  if (altitude <= 200) {
    state = "PCDEPLOYED";
  }
}


// chute_release (200m to 0m)
void chute_release() {
  if (parachute == 'N') {
    chuterelease();
    parachute = 'C';
  }
  data();
  if ((altitude - lastaltitude) < 1) { // ADD ACCELERATION CONDITION AND CHANGE ALTITUDE PARAMETERS!!
    state = "LANDED";
  }
}

// landed
void landed() {
  if (flag == 'N') {
    upright();
    delay(5000);
    flagdeploy();
    delay (5000);
    flag = 'M'; 
    buzzer(); //BUZZER ACTIVATES HERE
  }
  data();

}



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
          state = "READY";
        }
        else if (cmdarg == "OFF"){
          cmdecho = "CXOFF";
          state = "IDLE";
        }
        else {
          Serial.println("Invalid command received.");
        }
      }
      else if (cmd == "ST"){
        if (cmdarg == "GPS") {
          cmdecho = "STGPS";
          
          gpshour = myGNSS.getHour();
          gpsmin = myGNSS.getMinute();
          gpssec = myGNSS.getSecond();
          
          int dtime = round(millis()/1000);
  
          int dhours = int(floor(dtime / 3600));
          dtime = dtime - dhours * 3600;
          int dminutes = int(floor(dtime / 60));
          dtime = dtime - dminutes * 60;
          int dseconds = dtime;

          hours = gpshour - dhours;
          mins = gpsmin - dminutes;
          secs = gpssec - dseconds;

          if (secs < 0){
            secs += 60;
            mins--;
          }
          if (mins < 0){
            mins += 60;
            hours--;
          }
          if (hours < 0){
            hours += 24;
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

          hours = newhour - dhours;
          mins = newmin - dminutes;
          secs = newsec - dseconds;

          if (secs < 0){
            secs += 60;
            mins--;
          }
          if (mins < 0){
            mins += 60;
            hours--;
          }
          if (hours < 0){
            hours += 24;
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
          rocketrelease();
        }
        else if (cmdarg == "HS") {
          cmdecho = "ACTHS";
          shielddeploy1();
        }
        else if (cmdarg == "PC") {
          cmdecho = "ACTPC";
          chuterelease();
        }
        else if (cmdarg == "AB") {
          cmdecho = "ACTAB";
          buzzer();
        }
        else if (cmdarg == "LED") {
          cmdecho = "ACTLED";
          ledblink();
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

///////////////////////////////////// OVERALL SETUP /////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Wire.setSCL(22);
  Wire.setSDA(23);
  Wire.begin();
  
  Serial1.begin(9600);
  
  bmpsetup();
  bnosetup();
  samsetup();
  bmpconfigure();
 
  servo1.attach(15,0,3000); // need pin number // release mechanism // mosfet 33
  servo2.attach(32,0,3000); // need pin number // heat shield
  servo3.attach(14,0,3000); // need pin number // flag

  hours = myGNSS.getHour();
  mins = myGNSS.getMinute();
  secs = myGNSS.getSecond();
}


///////////////////////////////////// STATE LOOP /////////////////////////////////////
void loop() {
  if (state == "IDLE"){
    ;
  }
  else if (state == "READY"){
    readie();
  }
  else if (state == "ASCENDING"){
    ascending();
  }
  else if (state == "DESCENDING"){
    descending();
  }
  else if (state == "HSDEPLOYED"){
    shield_release();
  }
  else if (state == "PCDEPLOYED"){
    chute_release();
  }
  else if (state == "LANDED"){
    landed();
  }
  else {
    Serial.print("Case error.");
  }

  readcommands();

  ledblink(); // make sure this stays at the end of the loop
}
