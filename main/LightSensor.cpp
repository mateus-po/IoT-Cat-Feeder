#include "LightSensor.h"
#include <Arduino.h>


LightSensor::LightSensor(int lightSensorPin): lightSensorPin(lightSensorPin) {
  pinMode(lightSensorPin, INPUT);
}
float LightSensor::readLux() {
  analogReadResolution(10);
  float volts =  analogRead(lightSensorPin) * 5 / 1024.0;
  float amps = volts / 10000.0;
  float microamps = amps * 1000000;
  float lux = microamps * 2.0;

  return lux;
}