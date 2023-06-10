const int trig = 4;

void setup() {     
  Serial.begin(115200); 

  pinMode(trig, OUTPUT);
 // digitalWrite(trig, HIGH);      
}

//void startRecording() {
//  Serial.println("Recording...");
//  digitalWrite(trig, LOW); 
//  delay(1250);
//  digitalWrite(trig, HIGH);
//  Serial.println("Recording started.");
//}
//
//void stopRecording() {
//  Serial.println("Stopping the recording...");
//  digitalWrite(trig, LOW);
//  delay(1250);
//  digitalWrite(trig, HIGH);
//  delay(10);
//  digitalWrite(trig, LOW);
//  Serial.println("Recording stopped.");
//}

void loop(){
//  startRecording();
//  delay(60000);
//  stopRecording();
//  delay(1000000);

digitalWrite(trig,LOW);
//delay(1250);
delay(100);
digitalWrite(trig,HIGH);
delay(35000);

  
}
