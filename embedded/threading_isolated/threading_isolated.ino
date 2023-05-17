TaskHandle_t task1;
TaskHandle_t task2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  xTaskCreatePinnedToCore(
                    ledBlink,   /* Task function. */
                    "LED",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */   
  delay(500);
  xTaskCreatePinnedToCore(
                    serialMessaging,
                    "SERIAL",
                    10000,
                    NULL,
                    1,
                    &task2,
                    1);
}

void ledBlink(void * pvParameters) {
  while (true) {
    Serial.println("HIGH");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.println("LOW");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void serialMessaging(void * pvParameters) {
  while (true) {
    Serial.println("W");
    delay(700);
    Serial.println("L");
    delay(700);
  }
}

void loop() {;}
