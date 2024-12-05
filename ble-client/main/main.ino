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
    Serial.println("Client Connected");
  }

  void onDisconnect(BLEClient *pClient) {
    isConnectedToServer = false;
    Serial.println("Client Disconnected");
  }
};

static void onNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic1, uint8_t* pData1, size_t length, bool isNotify) {
    Serial.printf("%d\n", pData1[0]);
    Serial.print("\n");
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

    if (pRemoteCharacteristicRead != nullptr) {
      String readValue = pRemoteCharacteristicRead->readValue().c_str();
      Serial.println("Read value: " + readValue);
    } else {
      Serial.println("Failed to find read characteristic.");
    }

    if (pRemoteCharacteristicNotify != nullptr) {
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

  delay(2000);
}