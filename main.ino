#include <HTTPClient.h>
#include <WiFi.h>

const char* ssid = "realme 9 Pro+";
const char* password = "mojewifi";
int LED_BUILTIN = 2;
String host = "http://catfact.ninja";
String url = "/fact";

void setup(){
    Serial.begin(115200);
    delay(1000);

    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop(){
  if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
      return;
  }
  HTTPClient http;
  String serverPath = host + url;
  Serial.println(serverPath);
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  delay(5000);
}
