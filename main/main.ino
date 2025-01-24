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

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Arduino.h>

int LED_BUILTIN = 2;

#define SERVICE_UUID        "85192177-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_WRITE_SSID "85192178-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_WRITE_PASSWORD "85192179-2bbe-45b4-ac18-115c21bc8e0f"
 
const int len = 64; 
const uint32_t addressStart = 0x3F3000;
uint8_t FLASH_Address_SSID = 0;
uint8_t FLASH_Address_Password = 1;
 
char ssid[len]; // WiFi SSID
char password[len]; // WiFi Password
char user_id[len];
char alert_threshold[len];


const char* mqtt_broker = ""; // Broker's IP
const char* mqtt_user = "acf_user"; // MQTT user which is used for authentication
const char* mqtt_password = "admin123"; // MQTT password used for authenticating aforementioned user
const int mqtt_port = 1883; 

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 2;
const int  daylightOffset_sec = 3600;
float currentWeight = 0;


String client_id="8813bf0c3230";

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

void flashWrite(char data[len], int i) {
  uint32_t flashAddress = addressStart + i*len;
  char buff_write[len];
  strcpy(buff_write, data);
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_write, sizeof(buff_write)-1))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_write);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_write);
}

char* flashRead(int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  static char buff_read[len];
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_read, sizeof(buff_read)-1)) {
    return buff_read;
  } else  
    return "";  
}

void flashErase() {
  if (ESP.flashEraseSector(addressStart / 4096))
    Serial.println("\nErase [ok]");
  else
    Serial.println("\nErase [error]");
}

bool deviceConnected = false;

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected.");
  }
};

class WriteSSIDCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    
    if (value.length() > 0) {
      Serial.print("SSID: ");
      strcpy(ssid, value.c_str());
      Serial.println(ssid);
      flashErase();
      flashWrite(ssid, 0);  
      flashWrite(password, 1);
      flashWrite(user_id, 2);
      flashWrite(alert_threshold, 3);
      WiFi.begin(ssid, password);
      configTime(0, daylightOffset_sec, ntpServer);
    }
  }
};

class WritePasswordCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    
    if (value.length() > 0) {
      Serial.print("Password: ");
      strcpy(password, value.c_str());
      Serial.println(password);
      flashErase();
      flashWrite(ssid, 0);  
      flashWrite(password, 1);
      flashWrite(user_id, 2);
      flashWrite(alert_threshold, 3);
      WiFi.begin(ssid, password);
      configTime(0, daylightOffset_sec, ntpServer);
    }
  }
};


Speaker speaker(SPEAKER_PIN);
SSD1306_OLED oled(OLED_I2C_ADDRESS, OLED_WIDTH, OLED_HEIGHT, OLED_BRIGHTNESS);
LightSensor lightSensor(TEMT6000);
HX711 scale;
Motor motor(motor1Pin1, motor1Pin2, enable1Pin);

String get_register_topic() {
  return "ACF_app/" + client_id + "/action";
}

String get_topic_prefix() {
  return "ACF_app/" + String(user_id) + "/devices/" + client_id;
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
    Serial.println("Connecting to MQTT broker...");
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      
      String register_topic = get_register_topic();
      client.subscribe(register_topic.c_str());
      Serial.println("Subscribed to: " + register_topic);

      if (String(user_id).length() > 0) {
        String action_topic = get_action_topic();
        client.subscribe(action_topic.c_str());
        Serial.println("Subscribed to: " + action_topic);
      }
      
    } else {
      Serial.print("Failed. Error code: ");
      Serial.println(client.state());
    }
}

void publish_weight() {
 if (client.connected()) {
    float weight;
    if (scale.is_ready()) {
      weight = (float) scale.get_units(10) / 1146;
      if (weight < 0) weight = 0;
    } else {
      weight = 0.0;
    }
    currentWeight = weight;

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
  }
}

long int distributionStart =  0;

void distribute() {
  distributionStart = millis();
  speaker.playShortMelody();
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
          strcpy(user_id, new_user_id.c_str());
          Serial.print("Updated user_id to: ");
          Serial.println(user_id);

          flashErase();
          flashWrite(ssid, 0);  
          flashWrite(password, 1);
          flashWrite(user_id, 2);
          flashWrite(alert_threshold, 3);

          String action_topic = get_action_topic();
          client.subscribe(action_topic.c_str());
          Serial.println("Subscribed to: " + action_topic);
      }
    } else if (action == "distribute") {
      distribute();
    } else if (action == "update") {
      String settings = data["settings"]["weightThreshold"];
          strcpy(alert_threshold, String(settings).c_str());
          flashErase();
          flashWrite(ssid, 0);  
          flashWrite(password, 1);
          flashWrite(user_id, 2);
          flashWrite(alert_threshold, 3);
    } else {
      Serial.println("No valid action or user_id found in data");
    }
  } else {
    Serial.println("No data field found in JSON payload");
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000);  
  Serial.println(get_client_id());
  BLEDevice::init("ACF_Serwer"); 
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristicWriteSSID = pService->createCharacteristic(
    CHARACTERISTIC_WRITE_SSID,
    BLECharacteristic::BLECharacteristic::PROPERTY_WRITE
  );

  BLECharacteristic *pCharacteristicWritePassword = pService->createCharacteristic(
    CHARACTERISTIC_WRITE_PASSWORD,
    BLECharacteristic::PROPERTY_WRITE
  );


  pCharacteristicWriteSSID->setValue("ACF_001_WRITE_SSID");
  pCharacteristicWriteSSID->addDescriptor(new BLE2902());
  pCharacteristicWriteSSID->setCallbacks(new WriteSSIDCallback());

  pCharacteristicWritePassword->setValue("ACF_001_WRITE_PASSWORD");
  pCharacteristicWritePassword->addDescriptor(new BLE2902());
  pCharacteristicWritePassword->setCallbacks(new WritePasswordCallback());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();



  strcpy(ssid, flashRead(0));
  strcpy(password, flashRead(1));
  strcpy(user_id, flashRead(2));
  strcpy(alert_threshold, flashRead(3));

  Serial.printf("ssid: \"%s\"\npassword: \"%s\"\n", ssid, password);

  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(ssid, password);
  configTime(0, daylightOffset_sec, ntpServer);
  
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
const unsigned long publishInterval = 3000;
long int BLELastAdvertised = millis() - 2001, WiFiLastLogin = millis() - 2001, weightLastUpdate = millis() - 2001;
unsigned long lastReconnectAttempt =  millis() - 2001;


void displayWeight() {
  char result[32];

  if (scale.is_ready()) {
    float reading = (float) scale.get_units(10) / 1146;
    if (reading < 0) reading = 0;
    currentWeight = reading;
    dtostrf(reading, 8, 2, result);
    oled.clearDisplay();
    oled.drawRectangle(0, 0, 127, 31, true);
    oled.drawRectangle(2, 2, 125, 29, true);
    oled.drawText(5, 5, result, 2, true);
    oled.display();
  } 
  else {
    // Serial.println("HX711 not found.");
  }
}

void loop() {
  if (distributionStart+3000 > millis()) {
    speaker.fillBuffer();
    motor.moveForward();
    return;
  } else {
    motor.stopMotor();
  }

  if (millis() - weightLastUpdate >= 500) {
    weightLastUpdate =  millis();
    displayWeight();
  } 

  if(!deviceConnected && (BLELastAdvertised + 2000) < millis()) {
    BLELastAdvertised = millis();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  }

  unsigned long currentMillis = millis();
  if (String(user_id) != "tombstone" && currentMillis - lastPublishTime >= publishInterval) {
    lastPublishTime = currentMillis;
    publish_weight();
    publish_light();
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - WiFiLastLogin >= 4000) {
      WiFi.begin(ssid, password);
      WiFiLastLogin = millis();
    } 
    else if (millis() - WiFiLastLogin >= 2000) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      digitalWrite(LED_BUILTIN, LOW);
    }
    return;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    if (atoi(alert_threshold) > currentWeight) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }

  if (!client.connected()) {
    unsigned long now = millis();
    
  if (now - lastReconnectAttempt > 3000) {
      lastReconnectAttempt = now;
      connect_mqtt();
    }
  } else {
    client.loop();
  }

}


