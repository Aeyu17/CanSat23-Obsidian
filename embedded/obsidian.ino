


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
#include <vector>
#include <string.h>
#include <SoftwareSerial.h>


// led on for entire duration

// 700m
  // detaches from rocket
  // servo 1
  // ~10 degrees

// 500 meters 
  // releases heat shield
  // servo 2
  // smack talk the mechanical team for degrees

// 200m 
  // parachute deploys and heat shield retracts
  // servo 1 and servo 2
  // servo 1 rotates more and servo 2 rotates backwards 
  // smack talk mechanical team for degrees AGAIN

// 0m
  // heat shield releases and uprights
  // servo 2
  // smack talk mechanical team for degrees AGAIN

  // flag raises/rotates
  // turn on beacon
  // servo 3
  // smack talk mechanical team for degrees AGAIN



// initializing
int phase = 0;
String shield = "N";
String flag = "N";
String parachute = "N";
String modes = "S";
String state = "AWAITING LAUNCH";
int i;
const int ID = 1070;
const int GROUND = 0;
int packets = 0;
double alt_offset;

Servo servo1; // rocket and parachute
Servo servo2; // heat shield
Servo servo3; // flag

SFE_UBLOX_GNSS myGNSS;
long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.


Adafruit_BMP3XX bmp;

// define sea level pressure (will probably need to change)
#define SEALEVELPRESSURE_HPA (1017)


Adafruit_BNO055 bno = Adafruit_BNO055(55);




float gpstime = 0;



// SETUP
// BMP388
// BNO055
// SAM-M8Q
// OpenLog
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

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  
}




// led setup
void ledblink(void){
  pinMode(27,OUTPUT);

  while(true) {
    pinMode(27,HIGH);
    delay(500);
    pinMode(27,LOW);
    delay(500);
  }
}




// beginning of software for loop


// configuring the pressure to ground 
void bmpconfigure() {
 
  double number1;
  double number2;
  double number3;
  double number4;
  double number5;
 
  bmp.readAltitude(SEALEVELPRESSURE_HPA);
  delay(3000);
  
  for(int i=0;i<6;i++) {
    switch(i){
      case 1: number1 = bmp.readAltitude(SEALEVELPRESSURE_HPA); break;
      case 2: number2 = bmp.readAltitude(SEALEVELPRESSURE_HPA); break;
      case 3: number3 = bmp.readAltitude(SEALEVELPRESSURE_HPA); break;
      case 4: number4 = bmp.readAltitude(SEALEVELPRESSURE_HPA); break;
      case 5: number5 = bmp.readAltitude(SEALEVELPRESSURE_HPA); break;
      default: true; break;
    }
  }
  
  alt_offset = (number1 + number2 + number3 + number4 + number5)/5;
  Serial.print(alt_offset);
}



// initial rocket release
void rocketrelease() {
  servo1.write(10);
  delay(300);
}



// parachute release
void chuterelease() {
  servo1.write(120);
  delay(300);
}



// first shield deploy
void shielddeploy1() {
  servo2.write(20); // don't know this number yet
  delay(300);
}



// shield retract
void shieldretract() {
  servo2.write(-20); // don't know this number yet
  delay(300); 
}



// second shield deploy
void shielddeploy2() {
  servo2.write(60); // don't know this number yet
  delay(300);
}



// flag delpy
void flagdeploy() {
  servo3.write(500); // don't know this number yet
  delay(300);
}


// reading data
void data() {
  
  // bmp388 reading
  if (! bmp.performReading()) {
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
  long gpssat;
  long gpsalt;
  
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    
    gpslat = myGNSS.getLatitude();
    gpslong = myGNSS.getLongitude();
    gpsalt = myGNSS.getAltitude();
    gpssat = myGNSS.getSIV();
  }




  // time
  float currentTime = millis();
  currentTime = currentTime / 1000;
  int hourInt = currentTime / 3600;
  String hours = String(hourInt);
  if (hours.length() == 1){
    hours = "0" + hours;
  }
  int minuteInt = currentTime / 60 - hourInt * 60;
  String minutes = String(minuteInt);
  if (minutes.length() == 1){
    minutes = "0" + minutes;
  }
  String seconds = String(float(currentTime - hourInt * 3600 - minuteInt * 60));
  if (seconds.length() != 5){
    seconds = "0" + seconds;
  }
  String missiontime = hours + ":" + minutes + ":" + seconds;

  
  // voltage                                         
  int vread = analogRead(13);
  float voltage = vread * (3.3/1023.0) * 2;

  
  double temperature = bmp.temperature;
  double altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset;
  packets += 1;
  String c = ",";

// TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, 
// PC_DEPLOYED, MAST_RAISED, PRESSURE, TEMPERATURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, 
// GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO


  Serial.println(String(ID) + c + String(missiontime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslat) + c + String(gpslong) + c + String(gpssat) + 
  c + String(roll) + c + String(pitch) + c + String(cmdecho) + c +String(phase));
  
  // XBee
  Serial1.println(String(ID) + c + String(missiontime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslat) + c + String(gpslong) + c + String(gpssat) + 
  c + String(euler.x) + c + String(euler.y) + c + String(cmdecho) + c +String(phase));
}
// 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
// 1070,00:00:00,10000,F,S,ELEVENCHARS,700.1,P,C,M,100.1,3.1,00:00,100.1,1000.0001,1000.0001,10000,100.01,100.01,SP101325,0


// flight stages 
// flightstageone (700m to 500m)
void flightstageone(){
  data();
}
// flightstagetwo (500m to 200m)
// flightstagethree (200m to 0m)


void setup() {
Serial.begin(115200);
Wire.setSCL(22);
Wire.setSDA(23);
Wire.begin();

Serial1.begin(9600);

bmpsetup();
bnosetup();
samsetup();


servo1.attach(15,0,3000); // need pin number // release mechanism // mosfet 33
servo2.attach(32,0,3000); // need pin number // heat shield
servo3.attach(14,0,3000); // need pin number // flag



}



void loop() {




ledblink(); // make sure this stays at the end of the loop

}
