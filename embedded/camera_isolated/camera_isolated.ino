const int trig = 36;
const int led = LED_BUILTIN;

void setup() {                
  // initialize the digital pins as output.
  Serial.begin(115200);

  Serial.println("Setting the pin mode...");
  pinMode(led, OUTPUT);
  pinMode(trig, OUTPUT);   

  Serial.println("Writing to high...");   
  digitalWrite(led, HIGH);  
  digitalWrite(trig, LOW); 
}

// Hold HIGH and trigger quick (<250ms) LOW to take a photo. Holding LOW and trigger HIGH starts/stops video recording

void loop() {
  Serial.println("Taking a picture!");
  digitalWrite(trig, LOW);   
  digitalWrite(led, LOW);
  
  delay(50);               

  digitalWrite(trig, HIGH);    
  digitalWrite(led, HIGH);   
  Serial.println("Took a picture!");
  Serial.println("Waiting to take another picture...");

  delay(10000);  
}