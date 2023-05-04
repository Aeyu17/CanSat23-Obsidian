const int trig = 33;
const int led = LED_BUILTIN;


void setup() {     
  Serial.begin(115200); 

  delay(500);
  
  if (Serial)
  {
    Serial.println("WORKING");
  }
  
  // initialize the digital pins as output.
  pinMode(led, OUTPUT);
  pinMode(trig, OUTPUT);         

  digitalWrite(led, HIGH);
  digitalWrite(trig, LOW); 

  Serial.println("MADE IT HERE");

  delay(1000);

  // start recording
  Serial.println("Recording...");
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  
  delay(10000);

  Serial.println("HIGH");
  digitalWrite(trig, HIGH);
  delay(10);
  Serial.println("LOW");
  digitalWrite(trig, LOW);
}

void loop(){;}