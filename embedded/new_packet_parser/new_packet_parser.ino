
String packet;

void setup() {
  
Serial.begin(115200);
Serial1.begin(9600);

}

void loop() {
  packet = "";

if (Serial1.available()){
    packet = Serial1.readStringUntil('\n');
    packet = packet + "\n";
    Serial.println(packet);
}  
  delay(5000);
}
