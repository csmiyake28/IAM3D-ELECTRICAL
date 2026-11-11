#include <WiFi.h>

const char *ssid = "dingdong";
const char *password = "dingdong";

void setup() {
  // put your setup code here, to run once:
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

}

void loop() {
  // put your main code here, to run repeatedly:

}
