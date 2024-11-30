#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = ""; // WiFi SSID
const char* password = ""; // WiFi Password

const char* mqtt_broker = ""; // Broker's IP
const char* mqtt_user = ""; // MQTT user which is used for authentication
const char* mqtt_password = ""; // MQTT password used for authenticating aforementioned user
const int mqtt_port = 1883; 

String client_id;

String get_weight_topic() {
  return client_id + "/devices/weight";
}

String get_pressure_topic() {
  return client_id + "/devices/pressure";
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
    } else {
      Serial.print("Failed. Error code: ");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void publish_weight() {
  if (client.connected()) {
    float weight = generate_random_value(10.0, 40.0);
    char tempString[10];
    snprintf(tempString, sizeof(tempString), "%.2f", weight);

    String topic = get_weight_topic();
    client.publish(topic.c_str(), tempString); 

    Serial.print("Published weight: ");
    Serial.println(tempString);
  }
}

void publish_pressure() {
  if (client.connected()) {
    float pressure = generate_random_value(10.0, 40.0);
    char tempString[10];
    snprintf(tempString, sizeof(tempString), "%.2f", pressure);

    String topic = get_pressure_topic();
    client.publish(topic.c_str(), tempString); 

    Serial.print("Published pressure: ");
    Serial.println(tempString);
  }
}

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client_id = get_client_id();
  
  client.setServer(mqtt_broker, mqtt_port);
  connect_mqtt();
}

void loop() {
  if (!client.connected()) {
    connect_mqtt();
  }

  client.loop();
  publish_weight();
  publish_pressure();
  delay(3000);
}