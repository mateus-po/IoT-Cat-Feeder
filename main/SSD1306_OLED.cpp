#include "SSD1306_OLED.h"

SSD1306_OLED::SSD1306_OLED(uint8_t address, uint8_t width, uint8_t height, uint8_t brightness)
    : i2cAddress(address), displayWidth(width), displayHeight(height), displayBrightness(brightness) {
    buffer = new uint8_t[displayWidth * displayHeight / 8]();
}

SSD1306_OLED::~SSD1306_OLED() {
    delete[] buffer;
}

void SSD1306_OLED::begin() {
    Wire.begin();
    initializeDisplay();
    clearDisplay();
    display();
}

void SSD1306_OLED::initializeDisplay() {

    sendCommand(0xAE); // Display OFF
    sendCommand(0x20); // Set Memory Addressing Mode
    sendCommand(0x00); // Horizontal addressing mode
    sendCommand(0xB0); // Set Page Start Address for Page Addressing Mode
    sendCommand(0xC8); // COM Output Scan Direction
    sendCommand(0x00); // Set low column address
    sendCommand(0x10); // Set high column address
    sendCommand(0x40); // Set start line address
    sendCommand(0x81); // Set contrast control
    sendCommand(displayBrightness); // Use brightness parameter
    sendCommand(0xA1); // Set segment re-map
    sendCommand(0xA6); // Normal display
    sendCommand(0xA8); // Set multiplex ratio
    sendCommand(displayHeight - 1); // Use height parameter (multiplex ratio = height - 1)
    sendCommand(0xD3); // Set display offset
    sendCommand(0x00); // No offset
    sendCommand(0xD5); // Set display clock divide ratio/oscillator frequency
    sendCommand(0xF0); // Divide ratio
    sendCommand(0xD9); // Set pre-charge period
    sendCommand(0x22);
    sendCommand(0xDA); // Set com pins hardware configuration
    sendCommand((displayHeight == 32) ? 0x02 : 0x12); // Set pins config based on height (32 or 64)
    sendCommand(0xDB); // Set vcomh
    sendCommand(0x20); // 0.77xVcc
    sendCommand(0x8D); // Enable charge pump regulator
    sendCommand(0x14);
    sendCommand(0xAF); // Display ON
}

void SSD1306_OLED::sendCommand(uint8_t command) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(0x00); // Control byte for command
    Wire.write(command);
    Wire.endTransmission();
}

void SSD1306_OLED::sendData(uint8_t data) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(0x40); // Control byte for data
    Wire.write(data);
    Wire.endTransmission();
}

void SSD1306_OLED::clearDisplay() {
    memset(buffer, 0, displayWidth * displayHeight / 8);
}

void SSD1306_OLED::fillDisplay() {
    memset(buffer, 255, displayWidth * displayHeight / 8);
}

void SSD1306_OLED::drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool color) {
  if (x1 >= x2 || y1 >= y2) return;

  for (uint8_t i = x1; i <= x2; i++) {
    drawPixel(i, y1, color);
    drawPixel(i, y2, color);
  }

  for (uint8_t i = y1; i <= y2; i++) {
    drawPixel(x1, i, color);
    drawPixel(x2, i, color);
  }
}

void SSD1306_OLED::display() {
    for (uint8_t page = 0; page < (displayHeight / 8); page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x00);
        sendCommand(0x10);

        for (uint8_t col = 0; col < displayWidth; col++) {
            sendData(buffer[page * displayWidth + col]);
        }
    }
}

void SSD1306_OLED::drawPixel(uint8_t x, uint8_t y, bool color) {
    if (x >= displayWidth || y >= displayHeight) return;

    uint16_t index = x + (y / 8) * displayWidth;
    if (color) {
        buffer[index] |= (1 << (y % 8));
    } else {
        buffer[index] &= ~(1 << (y % 8));
    }
}

void SSD1306_OLED::drawChar8x8(uint8_t x, uint8_t y, char c, bool color) {
    if (c < 32 || c > 127) return;
    c -= 32;

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t column = font8x8[(uint8_t)c][i];
        for (uint8_t j = 0; j < 8; j++) {
            bool pixelToDraw = column & (1 << j);
            if (!color) pixelToDraw = !pixelToDraw;
            drawPixel(x + i, y + j, pixelToDraw);
        }
    }
}

void SSD1306_OLED::drawChar8x16(uint8_t x, uint8_t y, char c, bool color) {
    if (c < 32 || c > 127) return;
    c -= 32;

    for (uint8_t i = 0; i < 8; i++) {
        uint16_t column = font8x16[(uint8_t)c][i];
        for (uint8_t j = 0; j < 16; j++) {
            bool pixelToDraw = column & (1 << j);
            if (!color) pixelToDraw = !pixelToDraw;
            drawPixel(x + i, y + j, pixelToDraw);
        }
    }
}

void SSD1306_OLED::drawChar16x16(uint8_t x, uint8_t y, char c, bool color) {
    if (c < 32 || c > 127) return;
    c -= 32;

    for (uint8_t i = 0; i < 16; i++) {
        uint16_t column = font16x16[(uint8_t)c][i];
        for (uint8_t j = 0; j < 16; j++) {
            bool pixelToDraw = column & (1 << j);
            if (!color) pixelToDraw = !pixelToDraw;
            drawPixel(x + i, y + j, pixelToDraw);
        }
    }
}


void SSD1306_OLED::drawText(uint8_t x, uint8_t y, const char *text, uint8_t font, bool color) {
    uint8_t step;

    if (font == 1) {
        step = 8;
    } else if (font == 2) {
        step = 8;             
    } else if (font == 3) {
        step = 16;      
    } else {
        return;
    }
    while (*text) {
        if (font == 1) {
          drawChar8x8(x, y, *text, color);
        } else if (font == 2) {
          drawChar8x16(x, y, *text, color);        
        } else if (font == 3) {
          drawChar16x16(x, y, *text, color);        
        }

        x += step;

        ++text;
    }
}

void SSD1306_OLED::displayDemo() {
  clearDisplay();
  for (uint8_t y = 0; y < displayHeight / 2; y += 3) {
    drawRectangle(y,y,displayWidth - 1 - y, displayHeight - 1 - y, true);
    display();
    delay(300);
  }

  fillDisplay();
  display();
  delay(300);
  for (uint8_t y = 1; y < displayHeight / 2; y += 3) {
    drawRectangle(y,y,displayWidth - 1 - y, displayHeight - 1 - y, false);
    display();
    delay(300);
  }

  clearDisplay();
  drawText(0,0,"Hello",2,true);
  display();
  delay(100);
  drawText(0,16,"Hello",3,true);
  display();
  delay(100);
  drawText(5*16,0,"World!",1, true);
  display();
  delay(100);
  drawText(5*16,8,"World!",1, true);
  display();
  delay(100);
  drawText(5*16,16,"World!",1, true);
  display();
  delay(100);
  drawText(5*16,24,"World!",1, true);
  display();
  delay(100);
  display();
  delay(500);

  fillDisplay();
  delay(500);

  drawText(0,0,"Hello",2,false);
  display();
  delay(100);
  drawText(0,16,"Hello",3,false);
  display();
  delay(100);
  drawText(5*16,0,"World!",1, false);
  display();
  delay(100);
  drawText(5*16,8,"World!",1, false);
  display();
  delay(100);
  drawText(5*16,16,"World!",1, false);
  display();
  delay(100);
  drawText(5*16,24,"World!",1, false);
  display();
  delay(100);
  display();
  delay(500);
}
