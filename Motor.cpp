#include "Motor.h"
#include <Arduino.h>

Motor::Motor(int motor1Pin1, int motor1Pin2, int enable1Pin): motor1Pin1(motor1Pin1), motor1Pin2(motor1Pin2), enable1Pin(enable1Pin) {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
}

void Motor::moveForward(){
  digitalWrite(enable1Pin, HIGH);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
}

void Motor::moveBackward(){
  digitalWrite(enable1Pin, HIGH);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
}

void Motor::stopMotor(){
  digitalWrite(enable1Pin, LOW);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
}