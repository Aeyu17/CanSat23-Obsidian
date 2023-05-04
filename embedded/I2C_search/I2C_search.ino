#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("I2C search");
  Serial.println("-------------------------");
  // 0x77 is the BMP
  // 0x28 is the BNO
  // 0x42 is the SAM

  Wire.begin();
  int i2c = 0b0000000;
  delay(3000);
  while (true){
    i2c = 0b0000000;
    for (int i = 0; i < 127; i++)
    {
      Serial.print("Search at [");
      Serial.print(i2c, HEX);
      Serial.print("]: ");
      Wire.beginTransmission(i2c);
      byte busStatus = Wire.endTransmission();
      if (busStatus == 0)
      {
         Serial.println("FOUND!");
      }
      else
      {
        Serial.println("not found");
      }
      i2c++;
    }
  }
}

void loop() {}
