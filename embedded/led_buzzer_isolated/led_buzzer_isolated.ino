const int ledPin = 12;
const int buzPin = 21;

void setup() {
  pinMode(ledPin,OUTPUT);

  pinMode(buzPin, OUTPUT);

}

void loop() {
  ledBlink();
  delay(500);
  buzzer();
}

void ledBlink() {
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}

void buzzer() {
  digitalWrite(buzPin, HIGH);
  delay(1000);
  digitalWrite(buzPin, LOW);
  delay(1000);
}