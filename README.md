## MQTT + BLE

## Intro

Clone repository using `PPrzybysz/MQTT-and-BLE` branch, please make required adjustments (both in `main/main.ino` and `mqtt.py` files), where specified in comment. 
You need to put your own WiFi configuration, same as MQTT broker configuration. Please note, that whole MQTT server configuration has been already placed in both `config/mosquitto.conf` and `docker-compose.yml`.

## How to run MQTT?

* After required adjustments, please run `docker compose up -d` (detach mode will allow you to work in the same terminal).
* You need to execute mosquitto script `mosquitto_passwd -c mosquitto/config/password.txt <user_name>` inside the docker container. Please make sure that the `password.txt` on your local machine 
reflects the provided configuration, since the volume is binded.
* Connect ESP32 development board to the same WiFi in which your local machine works. Please make sure that the docker network configuration uses the same network as your local machine.
* For HotSpots Broker's IP address will probably begin with `172.x.x.x`.
* Use correct naming for topics in `mqtt.py`, as they are using the MAC addresses of ESP32 development boards.

<img width="242" alt="Screenshot 2024-11-30 at 23 00 20" src="https://github.com/user-attachments/assets/412930d4-14ae-4ba6-9708-2437c258f11c">
<img width="242" alt="Screenshot 2024-11-30 at 23 00 10" src="https://github.com/user-attachments/assets/8d5a2c00-c110-47e3-95df-52f99fb96776">

## How to run BLE?

* Both directories `ble-client` and `ble-server` contain the `*.ino` files with the appropriate BLE logic for GATT client and GATT server respectively.
* The only requirement is to have bluetooth connection open and it should automatically work.
* You can use NRFConnect application in order to manually work with characteristic values (READ / WRITE / NOTIFY). 
