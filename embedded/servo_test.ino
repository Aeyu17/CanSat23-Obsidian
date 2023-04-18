#include <Servo.h>

static const int servoPin = 25;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
}

void loop() {
    servo1.write(180);
    delay(1000);
}
