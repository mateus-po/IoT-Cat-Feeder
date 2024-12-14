
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

#include "SSD1306_OLED.h"

SSD1306_OLED oled(OLED_I2C_ADDRESS, OLED_WIDTH, OLED_HEIGHT);

void setup() {
    oled.begin();
    oled.clearDisplay();
}

void loop() {
  oled.displayDemo();
}
