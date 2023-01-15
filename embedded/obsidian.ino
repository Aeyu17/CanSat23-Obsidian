#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Adafruit_LIS3MDL.h>
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
String state = "awaiting launch";
int i;
const int ID = 1070;
const int GROUND = 0;
int packets = 0;
double alt_offset;




Adafruit_BMP3XX bmp;
Adafruit_LSM6DSOX sox;
Adafruit_LIS3MDL lis3mdl;

// define sea level pressure (will probably need to change)
#define SEALEVELPRESSURE_HPA (1013.25)



// SETUP
// bmp388
// lsm6dsox
// lis3mdl

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
void soxsetup() {
  
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



// lis3mdl setup
void mdlsetup() {

}









void setup() {


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
  
  alt_offset = (num1 + num2 + num3 + num4 + num5)/5;
  Serial.print(alt_offset);
}



void loop() {


// 


}
