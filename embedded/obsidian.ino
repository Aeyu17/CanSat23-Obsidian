#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_config_keys.h>
#include <u-blox_structs.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Adafruit_LSM6DSOX.h>
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


Adafruit_BMP3XX bmp;
Adafruit_LSM6DSOX sox;

// define sea level pressure (will probably need to change)
#define SEALEVELPRESSURE_HPA (1017)




// SETUP
// bmp388
// lsm6dsox

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



// lsm6dsox setup
void dsoxsetup() {
  
Serial.println("Adafruit LSM6DSOX test");

  if (!sox.begin_I2C()) {
    Serial.println("Failed to find LSM6DSOX chip");
  }

  Serial.println("LSM6DSOX Found");

  // sox.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (sox.getAccelRange()) {
  case LSM6DS_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case LSM6DS_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case LSM6DS_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case LSM6DS_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  // sox.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS );
  Serial.print("Gyro range set to: ");
  switch (sox.getGyroRange()) {
  case LSM6DS_GYRO_RANGE_125_DPS:
    Serial.println("125 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  case ISM330DHCX_GYRO_RANGE_4000_DPS:
    break; // unsupported range for the DSOX
  }

  // sox.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  Serial.print("Accelerometer data rate set to: ");
  switch (sox.getAccelDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  // sox.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  Serial.print("Gyro data rate set to: ");
  switch (sox.getGyroDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }
}

void ledblink(void){
  pinMode(27,OUTPUT);

  while(true) {
    pinMode(27,HIGH);
    delay(500);
    pinMode(27,LOW);
    delay(500);
  }
}

void setup() {
Serial.begin(115200);
Wire.setSCL(22);
Wire.setSDA(23);
Wire.begin();

Serial1.begin(9600);

bmpsetup();
dsoxsetup();

servo1.attach(15,0,3000); // need pin number // release mechanism // mosfet 33
servo2.attach(32,0,3000); // need pin number // heat shield
servo3.attach(14,0,3000); // need pin number // flag



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
  servo2.write(20);
  delay(300);
}



// shield retract
void shieldretract() {
  servo2.write(-20); 
  delay(300); 
}



// second shield deploy
void shielddeploy2() {
  servo2.write(60);
  delay(300);
}



// flag delpy
void flagdeploy() {
  servo3.write(500);
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

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  sox.getEvent(&accel, &gyro, &temp);


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

  
  double roll = gyro.gyro.x;
  double pitch = gyro.gyro.y; 
  double temperature = bmp.temperature;
  double altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset;
  packets += 1;
  String c = ",";

// TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, 
// PC_DEPLOYED, MAST_RAISED, TEMPERATURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, 
// GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO


  Serial.println(String(ID) + c + String(missiontime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslong) + c + String(gpssat) + 
  c + String(roll) + c + String(pitch) + c + String(cmdecho) + c +String(phase));
  
  // XBee
  Serial1.println(String(ID) + c + String(missiontime) + c + String(packets) + c + String(modes) + c + String(state)+ 
  c + String(altitude) + c + String(shield) + c + String(parachute) + c + String(flag) + c + String(temperature) + 
  c + String(voltage) + c + String(gpstime) + c + String(gpsalt) + c + String(gpslong) + c + String(gpssat) + 
  c + String(roll) + c + String(pitch) + c + String(cmdecho) + c +String(phase));
}




// flight stages 
// flightstageone (700m to 500m)
// flightstagetwo (500m to 200m)
// flightstagethree (200m to 0m)




void loop() {




ledblink; // make sure this stays at the end of the loop

}
