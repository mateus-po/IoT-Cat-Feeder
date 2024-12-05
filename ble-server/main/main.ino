#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID        "85192177-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_READ_UUID "85192178-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_NOTIFY_UUID "85192179-2bbe-45b4-ac18-115c21bc8e0f"

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

class ReadCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.println("[Read] Value:");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }
      Serial.println();
    }
  }
};

class NotifyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.println("[Notify] Value:");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }
      Serial.println();
    }

    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();
  }
};


void setup() {
  Serial.begin(115200);
  Serial.println("BLE Serwer!");

  BLEDevice::init("ACF_Serwer");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristicRead = pService->createCharacteristic(
    CHARACTERISTIC_READ_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );

  BLECharacteristic *pCharacteristicNotify = pService->createCharacteristic(
    CHARACTERISTIC_NOTIFY_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );


  pCharacteristicRead->setValue("ACF_001_READ");
  pCharacteristicRead->addDescriptor(new BLE2902());
  pCharacteristicRead->setCallbacks(new ReadCallbacks());

  pCharacteristicNotify->setValue("ACF_001_NOTIFY");
  pCharacteristicNotify->addDescriptor(new BLE2902());
  pCharacteristicNotify->setCallbacks(new NotifyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void loop() {
  if(!deviceConnected) {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  }
  delay(2000);
}
