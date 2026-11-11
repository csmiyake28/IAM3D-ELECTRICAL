#define LED_PIN 33  // onboard red LED, active LOW

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32-CAM started");
}

void loop() {
  digitalWrite(LED_PIN, LOW);  // on
  delay(500);
  digitalWrite(LED_PIN, HIGH);  // off
  delay(500);
}
