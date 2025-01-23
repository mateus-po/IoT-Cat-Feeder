#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include "LightSensor.h"
#include "SSD1306_OLED.h"
#include "Speaker.h"
#include <Arduino.h>
#include "soc/rtc.h"
#include "HX711.h"
#include "Motor.h"
#include "time.h"

const char* ssid = ""; // WiFi SSID
const char* password = ""; // WiFi Password

const char* mqtt_broker = ""; // Broker's IP
const char* mqtt_user = ""; // MQTT user which is used for authentication
const char* mqtt_password = ""; // MQTT password used for authenticating aforementioned user
const int mqtt_port = 1883; 

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 2;
const int  daylightOffset_sec = 3600;

String user_id;
String client_id;

#define TEMT6000 34
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_BRIGHTNESS 0x01
#define SPEAKER_PIN 25
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14; 


Speaker speaker(SPEAKER_PIN);
SSD1306_OLED oled(OLED_I2C_ADDRESS, OLED_WIDTH, OLED_HEIGHT, OLED_BRIGHTNESS);
LightSensor lightSensor(TEMT6000);
HX711 scale;
Motor motor(motor1Pin1, motor1Pin2, enable1Pin);

String get_register_topic() {
  return "ACF_app/" + client_id + "/action";
}

String get_topic_prefix() {
  return "ACF_app/" + user_id + "/devices/" + client_id;
}

String get_weight_topic() {
  return get_topic_prefix() + "/sensors/weight";
}

String get_light_topic() {
  return get_topic_prefix() + "/sensors/light";
}

String get_action_topic() {
  return get_topic_prefix() + "/action";
}

WiFiClient espClient;
PubSubClient client(espClient);

String get_client_id() {
  uint8_t mac[6];
  WiFi.macAddress(mac);

  String client_id = "";
  for (int i = 0; i < 6; i++) {
    client_id += String(mac[i], HEX);
  }

  return client_id;
}

void setup_wifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Wi-Fi...");
  }

  Serial.println("Connected to Wi-Fi");
}

float generate_random_value(float min_val, float max_val) {
  return min_val + (float) random(1000) / 1000.0 * (max_val - min_val);
}

void connect_mqtt() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      
      String register_topic = get_register_topic();
      client.subscribe(register_topic.c_str());
      Serial.println("Subscribed to: " + register_topic);

      if (user_id.length() > 0) {
        String action_topic = get_action_topic();
        client.subscribe(action_topic.c_str());
        Serial.println("Subscribed to: " + action_topic);
      }
      
    } else {
      Serial.print("Failed. Error code: ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void publish_weight() {
 if (client.connected()) {
    float weight;
    if (scale.is_ready()) {
      weight = (float) scale.get_units(10) / 1146;
    } else {
      weight = 0.0;
    }

    struct tm timeinfo;
    
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    char timeString[120];
    strftime(timeString, sizeof(timeString), "%B %d %Y %H:%M:%S", &timeinfo);

    StaticJsonDocument<128> jsonDoc;
    jsonDoc["value"] = weight;
    jsonDoc["timestamp"] = timeString;

    char jsonBuffer[128];
    serializeJson(jsonDoc, jsonBuffer);

    String topic = get_weight_topic();
    client.publish(topic.c_str(), jsonBuffer); 

    Serial.print("Published weight: ");
    Serial.println(jsonBuffer);
  }
}

void publish_light() {
 if (client.connected()) {
    float light = lightSensor.readLux();

    struct tm timeinfo;
    
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    char timeString[120];
    strftime(timeString, sizeof(timeString), "%B %d %Y %H:%M:%S", &timeinfo);

    StaticJsonDocument<128> jsonDoc;
    jsonDoc["value"] = light;
    jsonDoc["timestamp"] = timeString;

    char jsonBuffer[128];
    serializeJson(jsonDoc, jsonBuffer);

    String topic = get_light_topic();
    client.publish(topic.c_str(), jsonBuffer); 

    Serial.print("Published light: ");
    Serial.println(jsonBuffer);
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, message);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject data = jsonDoc["data"];
  if (!data.isNull()) {
    String action = data["action"];
    
    if (action == "register") {
      String new_user_id = data["userId"];
      if (new_user_id.length() > 0) {
          user_id = new_user_id;
          Serial.print("Updated user_id to: ");
          Serial.println(user_id);

          String action_topic = get_action_topic();
          client.subscribe(action_topic.c_str());
          Serial.println("Subscribed to: " + action_topic);
      }
    } else if (action == "distribute") {
      Serial.println("Distribution...");
    } else if (action == "update") {
      String settings = data["settings"];
      Serial.println(settings);
    } else {
      Serial.println("No valid action or user_id found in data");
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client_id = get_client_id();

  delay(1000);
  uint8_t baseMac[6];
  WiFi.macAddress(baseMac);
  Serial.println("MAC:");
  Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);

  Serial.println("Client ID:");
  Serial.println(client_id);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqtt_callback);
  connect_mqtt();

  oled.begin();
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(1000);
  scale.set_scale();    
  scale.tare();
}

unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 1000;

void loop() {
  if (!client.connected()) {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > 500) {
      lastReconnectAttempt = now;
      connect_mqtt();
    }
  } else {
    client.loop();
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastPublishTime >= publishInterval) {
    lastPublishTime = currentMillis;
    publish_weight();
    publish_light();
  }
}

