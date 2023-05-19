TaskHandle_t task1;
TaskHandle_t task2;

int num = 1;

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
                    (void*)&num,
                    1,
                    &task2,
                    1);
}

void ledBlink(void * parameters) {
  while (true) {
    Serial.println("HIGH");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.println("LOW");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void serialMessaging(void * parameters) {
  while (true) {
    //Serial.println(*((int*)parameters));
    int temp = *((int*)parameters);
    Serial.println(temp);
    temp++;
    Serial.println(temp);
    num++;
    Serial.println("W");
    delay(700);
    Serial.println("L");
    delay(700);
  }
}

void loop() {;}
