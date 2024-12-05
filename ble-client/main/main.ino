#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID        "85192177-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_READ_UUID "85192178-2bbe-45b4-ac18-115c21bc8e0f"
#define CHARACTERISTIC_NOTIFY_UUID "85192179-2bbe-45b4-ac18-115c21bc8e0f"

BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristicRead;
BLERemoteCharacteristic* pRemoteCharacteristicNotify;

bool isConnectedToServer = false;

class ClientCallbacks: public BLEClientCallbacks {
  void onConnect(BLEClient *pClient) {
    isConnectedToServer = true;
    Serial.println("Client connected.");
  }

  void onDisconnect(BLEClient *pClient) {
    isConnectedToServer = false;
    Serial.println("Client disconnected.");
  }
};

static void onNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic1, uint8_t* pData, size_t length, bool isNotify) {
    String receivedString = String((char*)pData, length); 
    Serial.print("[FROM NOTIFY]: ");
    Serial.print(receivedString);
    Serial.println();
}

void scanForDevices() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  Serial.println("Scanning for devices...");

  BLEScanResults* foundDevices = pBLEScan->start(3, false);

  for (int i = 0; i < foundDevices->getCount(); i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      Serial.println("Found our service!");
      connectToServer(&device);
      break;
    }
  }
  pBLEScan->stop();
}

void connectToServer(BLEAdvertisedDevice* device) {
  pClient = BLEDevice::createClient();
  Serial.println("Connecting to server...");

  pClient->setClientCallbacks(new ClientCallbacks());
  pClient->connect(device);

  if (pClient->isConnected()) {
    Serial.println("Connected to server!");

    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr) {
      Serial.println("Failed to find service.");
      return;
    }

    pRemoteCharacteristicRead = pRemoteService->getCharacteristic(CHARACTERISTIC_READ_UUID);
    pRemoteCharacteristicNotify = pRemoteService->getCharacteristic(CHARACTERISTIC_NOTIFY_UUID);

    if (pRemoteCharacteristicRead != nullptr && pRemoteCharacteristicRead->canRead()) {
      String readValue = pRemoteCharacteristicRead->readValue().c_str();
      Serial.println("Read value: " + readValue);
    } else {
      Serial.println("Failed to find read characteristic.");
    }

    if (pRemoteCharacteristicNotify != nullptr && pRemoteCharacteristicNotify->canNotify()) {
      pRemoteCharacteristicNotify->registerForNotify(onNotifyCallback);
      Serial.println("Subscribed to notifications.");
    } else {
      Serial.println("Failed to find notify characteristic.");
    }
  } else {
    Serial.println("Failed to connect to server.");
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("BLE Client!");

  BLEDevice::init("");
  scanForDevices();
}

void loop() {
  if (!isConnectedToServer) {
    scanForDevices();
    return;
  }

  if (isConnectedToServer && pRemoteCharacteristicNotify->canWrite()) {
    String colors[6] = {"RED", "BLUE", "GREEN", "BLACK", "ORANGE", "WHITE"};
    int randomIndex = random(0, 6);
    pRemoteCharacteristicNotify->writeValue(colors[randomIndex].c_str(), sizeof(colors[randomIndex].c_str()));
  }

  delay(2000);
}