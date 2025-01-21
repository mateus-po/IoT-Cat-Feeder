#include "LightSensor.h"
#include "SSD1306_OLED.h"
#include "Speaker.h"
#include <Arduino.h>
#include "soc/rtc.h"
#include "HX711.h"
#include "Motor.h"


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


void setup() {
  Serial.begin(115200);
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

void loop() {
  // char result[32];

  // if (scale.is_ready()) {
  //   float reading = (float) scale.get_units(10) / 1146;
  //   dtostrf(reading, 8, 2, result);
  //   oled.clearDisplay();
  //   oled.drawText(0, 0, result, 2, true);
  //   oled.display();
  // } 
  // else {
  //   Serial.println("HX711 not found.");
  // }
  // delay(1000);

  speaker.fillBuffer();
  speaker.playShortMelody();

}
