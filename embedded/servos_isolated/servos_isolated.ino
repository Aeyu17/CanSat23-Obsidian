#include <Servo.h>

Servo releaseServo;
Servo panelServo;
Servo flagServo;

const int rServoPin = 32;
const int pServoPin = 14;
const int fServoPin = 15;
const int mosfetPin = 27;

const int delayTime = 3000;

int panelPosition = 2;

// 50 is clockwise, closing
// 93 is stop
// 130 is counter clockwise, opening

void setup() {
  Serial.begin(115200);

  pinMode(mosfetPin, OUTPUT);
  digitalWrite(mosfetPin, HIGH);

  panelServo.attach(pServoPin);
  releaseServo.attach(rServoPin);
  flagServo.attach(fServoPin);
}

void loop() {
  // raiseFlag();
  // delay(delayTime);
  // lowerFlag();
  // delay(delayTime);

  // releaseContainer();
  // delay(delayTime);
  // releaseParachute();
  // delay(delayTime);
  // resetRelease();
  // delay(delayTime);

  // Serial.println("Going to Pos 1, opening");
  // openPanels();
  // delay(delayTime);
  Serial.println("Going to Pos 0, closing"); 
  closePanels();
  delay(delayTime);
  panelPosition = 2;
  // Serial.println("Going to Pos 2, opening");
  // upright();
  // delay(delayTime);
  // Serial.println("Going to Pos 0, closing");
  // closePanels();
  // delay(delayTime);
  // Serial.println("Going to Pos 1, opening");
  // openPanels();
  // delay(delayTime);
  // Serial.println("Going to Pos 2, opening");
  // upright();
  // delay(delayTime);
  // Serial.println("Going to Pos 0, closing");
  // closePanels();
  // delay(delayTime + delayTime);

  // raiseFlag();
  // delay(delayTime);
  // releaseParachute();
  // delay(delayTime);
  // upright();
  // delay(delayTime);
  // resetMechanisms();
  // delay(delayTime);
  
}

void raiseFlag() {
  Serial.println("Raising the flag...");
  flagServo.write(180);
}

void lowerFlag() {
  Serial.println("Lowering the flag...");
  flagServo.write(0);
}

void releaseContainer() {
  Serial.println("Releasing from the container...");
  releaseServo.write(90);
}

void releaseParachute() {
  Serial.println("Releasing the parachute...");
  releaseServo.write(180);
}

void resetRelease() {
  Serial.println("Resetting the release mechanism...");
  releaseServo.write(0);
}

// 130 closes, 50 opens, 93 stops
void openPanels() {
  Serial.println("Opening the heat shield...");
  switch (panelPosition) {
    case 0:
    panelServo.write(130);
    delay(8000);
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
}

void closePanels() {
  Serial.println("Closing the heat shield...");
  switch (panelPosition) {
    case 1:
    panelServo.write(50);
    delay(8000);
    panelServo.write(93);
    break;

    case 2:
    panelServo.write(50);
    delay(18000);
    panelServo.write(93);
    break;

    case 0:
    default:
    Serial.println("The panels are already closed!");
    break;
  }
  panelPosition = 0;
  Serial.println("The heat shield is closed.");
}

void upright() {
  Serial.println("Opening the heat shield...");
  switch (panelPosition) {
    case 0:
    panelServo.write(130);
    delay(25000);
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
}

void resetMechanisms() {
  Serial.println("Resetting the mechanisms...");
  closePanels();

  resetRelease();

  lowerFlag();
  Serial.println("All mechanisms are reset.");
}
