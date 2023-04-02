#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define SEALEVELPRESSURE_HPA (1013.25) // predefined by BMP, change as needed

Adafruit_BMP3XX bmp; // I2C address 0x77
SFE_UBLOX_GNSS myGNSS; // I2C address 0x42
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); // I2C address 0x28

// BMP
float temperature;
float altitude;
float pressure;
float alt_offset;

// SAM
float latitude;
float longitude;
float gps_altitude;
int siv; // satellites in view
String gps_time;

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
}


void updateData() {
  /* 
   *  Updates the internal variables with the newly read data
   *  Returns none
   */

  // BMP
  temperature = round(10 * bmp.temperature)/10.0;
  pressure = round(bmp.pressure / 100.0)/10.0;
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
  voltage = analogRead(A2)*(6.6/8192)*2;

  // Mission Time
  int dtime = round(millis()/1000);
  int dhours = int(floor(dtime / 3600));
  dtime = dtime - dhours * 3600;
  int dminutes = int(floor(dtime / 60));
  dtime = dtime - dminutes * 60;
  int dseconds = dtime;
  
  String(startHour + dhours) + ":" + String(startMinute + dminutes) + ":" + String(startSecond + dseconds);
  
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

String packetGenerator(){
  /*
   * Creates a packet to be sent down to the GCS
   * Returns the packet
   */
   packetCount++;
   
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

void loop() {
  updateData();
  String packet = packetGenerator();
  debugPrintData();
}
