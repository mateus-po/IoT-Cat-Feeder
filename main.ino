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
const uint32_t addressStart = 0x3E0000; 
uint8_t FLASH_Address_SSID = 0;
uint8_t FLASH_Address_Password = 1;
 
char ssid[len], password[len];

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
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.print("SSID: ");
      strcpy(ssid, value.c_str());
      Serial.println(ssid);
      flashErase();
      flashWrite(ssid, 0);  
      flashWrite(password, 1);
      WiFi.begin(ssid, password);
    }
  }
};

class WritePasswordCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.print("Password: ");
      strcpy(password, value.c_str());
      Serial.println(password);
      flashErase();
      flashWrite(ssid, 0);  
      flashWrite(password, 1);
      WiFi.begin(ssid, password);
    }
  }
};


void setup() {
  Serial.begin(115200);

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
  Serial.printf("ssid: \"%s\"\npassword: \"%s\"\n", ssid, password);

  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(ssid, password);
}

long int BLELastAdvertised = millis() - 2001, WiFiLastLogin = millis() - 2001;

void loop() {
  if(!deviceConnected && (BLELastAdvertised + 2000) < millis()) {
    BLELastAdvertised = millis();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  }

  if (WiFi.status() != WL_CONNECTED) {
    if ((WiFiLastLogin+2000) < millis()) {
      WiFi.begin(ssid, password);
      WiFiLastLogin = millis();
    } 
    else if ((WiFiLastLogin+1000) > millis()) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      digitalWrite(LED_BUILTIN, LOW);
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
