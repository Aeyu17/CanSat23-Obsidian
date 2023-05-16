const int trig = 4;

void setup() {     
  Serial.begin(115200); 

  pinMode(trig, OUTPUT);         

  startRecording();
  delay(10000);
  stopRecording();


}

void startRecording() {
  Serial.println("Recording...");
  digitalWrite(trig, LOW); 
  delay(1000);
  digitalWrite(trig, HIGH);
  Serial.println("Recording started.");
}

void stopRecording() {
  Serial.println("Stopping the recording...");
  digitalWrite(trig, LOW);
  delay(1000);
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  Serial.println("Recording stopped.");
}

void loop(){;}
