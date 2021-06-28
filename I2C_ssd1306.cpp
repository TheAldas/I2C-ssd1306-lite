#include "I2C_ssd1306.h"

#if __AVR__
#include <avr/pgmspace.h>
#endif


I2C_ssd1306::I2C_ssd1306(uint8_t width, uint8_t height, byte ssd1306_address) {
  _width = width;
  _height = height;
  _addr = ssd1306_address;
  _screenBuffer = (uint8_t *)malloc((width * (height + 7) / 8));
}



void I2C_ssd1306::begin(TwoWire &I2Cwire) {
  wire = &I2Cwire;
  initialize();
}

void I2C_ssd1306::display() {
  uint8_t addrResList[] = {
    COMMAND_SET_PAGE_ADDRESS,
    0x00, _height - 1,
    COMMAND_SET_COLUMN_ADDRESS,
    0x00, (_width - 1)
  };

  sendCommandList(addrResList, sizeof(addrResList));
  free(addrResList);
  uint16_t columnsCount = (_width * _height / 8);
  uint8_t bytesSent = 1;
  uint8_t *ptr = _screenBuffer;
  START_TRANSMISSION
  wire->write(dataByte);
  while (columnsCount--) {
    if (bytesSent >= MAX_I2C_BYTES) {
      END_TRANSMISSION
      START_TRANSMISSION
      wire->write(dataByte);
      bytesSent = 1;
    }
    wire->write(*ptr++);
    bytesSent++;
  }
  END_TRANSMISSION
}

void I2C_ssd1306::clearDisplay() {
  memset(_screenBuffer, 0, (_width * (_height + 7) / 8));
}

void I2C_ssd1306::drawPixel(int16_t x, int16_t y, uint8_t color) {
  if (x >= _width || y >= _height || x < 0 || y < 0) return;
  switch (color) {
    case COLOR_BLACK:
      _screenBuffer[((int)((y >> 3) * (_width)) + x)] &= ~(1 << (y & 0b111));
      break;
    case COLOR_WHITE:
      _screenBuffer[((int)((y >> 3) * (_width)) + x)] |= (1 << (y & 0b111));
      break;
    default:
      _screenBuffer[((int)((y >> 3) * (_width)) + x)] ^= (1 << (y & 0b111));
      break;

  }
}

void I2C_ssd1306::fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color){
  width--;
  for(uint8_t i = 0; i < height; i++){
    drawHLine(x, y + i, x + width, color);
  }
}

void I2C_ssd1306::fillRectRound(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t cornerRadius, uint8_t color){
  if(width < 1 || height < 1) return;
  width--;
  height--;
  
  fillCircleQuarter(x + width - cornerRadius, y + cornerRadius, cornerRadius, 0, color);
  fillCircleQuarter(x + cornerRadius, y + cornerRadius, cornerRadius, 1, color);
  fillCircleQuarter(x + cornerRadius, y + height - cornerRadius, cornerRadius, 2, color);
  fillCircleQuarter(x + width - cornerRadius, y + height - cornerRadius, cornerRadius, 3, color);

  for(uint8_t i = 0; i < height + 1; i++){
    if(cornerRadius >= i || height - cornerRadius <= i) drawHLine(x + cornerRadius + 1 , y + i , x + width- cornerRadius - 1, color); 
    else drawHLine(x, y + i, x + width, color);
  }
}

void I2C_ssd1306::fillCircle(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t color){
  uint32_t x = radius, y = 0, radiusThreshold = radius * radius + radius;
  drawHLine(midX - x, midY, midX + x, color);

  while (x > y) {
    y++;

    if (radiusThreshold < (x * x + y * y)) {

      drawHLine(midX - y + 1, midY - x, midX + y - 1, color);
      drawHLine(midX - y + 1, midY + x, midX + y - 1, color);
      
      x--;
    }
    if(x < y) break;
    
    drawHLine(midX - x, midY + y, midX + x, color);
    drawHLine(midX - x, midY - y, midX + x, color);
  }
}

/*circle quarters:
 |---|---|
 | 1 | 0 |
 |---|---|
 | 2 | 3 |
 |---|---|
*/
void I2C_ssd1306::fillCircleQuarter(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t quarter, uint8_t color){
  uint32_t x = radius, y = 0, radiusThreshold = radius * radius + radius;
  if(quarter == 0 || quarter == 3) drawHLine(midX, midY, midX + x, color);
  else drawHLine(midX - x, midY, midX, color);

  while (x > y) {
    y++;

    if (radiusThreshold < (x * x + y * y)) {
      switch (quarter)
      {
        case 0:
          drawHLine(midX, midY - x, midX + y - 1, color);
          break;
        case 1:
          drawHLine(midX - y + 1, midY - x, midX, color);
          break;
        case 2:
          drawHLine(midX - y + 1, midY + x, midX, color);
          break;
        case 3:
          drawHLine(midX, midY + x, midX + y - 1, color);
          break;
        default:
          break;
      } 
      x--;
    }
    if(x < y) break;

    switch (quarter)
    {
      case 0:
        drawHLine(midX, midY - y, midX + x, color);
        break;
      case 1:
        drawHLine(midX - x, midY - y, midX, color);
        break;
      case 2:
        drawHLine(midX - x, midY + y, midX, color);
        break;
      case 3:
        drawHLine(midX, midY + y, midX + x, color);
        break;
      default:
        break;
    }
  }
}

void I2C_ssd1306::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color){
  drawHLine(x, y, x + width - 1, color);
  drawVLine(x + width - 1, y + 1, y + height - 1, color);
  drawHLine(x, y + height - 1, x + width - 2, color);
  drawVLine(x, y + 1, y + height - 2, color);
}

void I2C_ssd1306::drawRectRound(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t cornerRadius, uint8_t color){
  if(width < 1 || height < 1) return;
  width--;
  height--;
  
  drawCircleQuarter(x + width - cornerRadius, y + cornerRadius, cornerRadius, 0, color);
  drawCircleQuarter(x + cornerRadius, y + cornerRadius, cornerRadius, 1, color);
  drawCircleQuarter(x + cornerRadius, y + height - cornerRadius, cornerRadius, 2, color);
  drawCircleQuarter(x + width - cornerRadius, y + height - cornerRadius, cornerRadius, 3, color);

  drawHLine(x + cornerRadius + 1, y, x + width - cornerRadius, color);
  drawVLine(x + width, y  + cornerRadius + 1, y + height - cornerRadius - 1, color);
  drawHLine(x + cornerRadius + 1, y + height, x + width - cornerRadius, color);
  drawVLine(x, y + cornerRadius + 1, y + height  - cornerRadius - 1, color);
}

void I2C_ssd1306::drawCircle(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t color) {
  //calculating only one quarter of the circle until x is y
  //decreasing x everytime if x^2 + y^2 > r^2 + r
  //r^2 + r will be a 'radiusThreshold' for now, I don't know how to call it properly, cause I've come up with the formula
  //it might be similar to mid point circle drawing algorithm
  uint32_t x = radius, y = 0, radiusThreshold = radius * radius + radius;
  drawPixel(midX + radius, midY, color);
  if (radius != 0) {
    drawPixel(midX, midY + radius, color);
    drawPixel(midX, midY - radius, color);
    drawPixel(midX - radius, midY, color);
  }

  while (x > y) {
    y++;

    if (radiusThreshold < (x * x + y * y)) {
    x--;
    }
    if(x < y) break;

    drawPixel(midX + x, midY + y, color);
    drawPixel(midX + x, midY - y, color);
    drawPixel(midX - x, midY + y, color);
    drawPixel(midX - x, midY - y, color);
    if (x != y) {
      drawPixel(midX + y, midY + x, color);
      drawPixel(midX + y, midY - x, color);
      drawPixel(midX - y, midY + x, color);
      drawPixel(midX - y, midY - x, color);
    }
  }
}

/*circle quarters:
 |---|---|
 | 1 | 0 |
 |---|---|
 | 2 | 3 |
 |---|---|
*/
void I2C_ssd1306::drawCircleQuarter(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t quarter, uint8_t color){
  uint32_t x = radius, y = 0, radiusThreshold = radius * radius + radius;
  
  if (radius != 0) {
    switch (quarter)
    {
    case 0:
      drawPixel(midX + radius, midY, color);
      drawPixel(midX, midY - radius, color);
      break;
    case 1:
      drawPixel(midX, midY - radius, color);
      drawPixel(midX - radius, midY, color);
      break;
    case 2:
      drawPixel(midX - radius, midY, color);
      drawPixel(midX, midY + radius, color);
      break;
    case 3:
      drawPixel(midX, midY + radius, color);
      drawPixel(midX + radius, midY, color);
      break;
    default:
      break;
    }
  }else{
    drawPixel(midX, midY, color);
  }

  while (x > y) {
    y++;

    if (radiusThreshold < (x * x + y * y)) {
    x--;
    }

    if(x < y) break;

    switch (quarter)
    {
    case 0:
      drawPixel(midX + x, midY - y, color);
      if (x != y) drawPixel(midX + y, midY - x, color);
      break;
    case 1:
      drawPixel(midX - x, midY - y, color);
      if (x != y) drawPixel(midX - y, midY - x, color);
      break;
    case 2:
      drawPixel(midX - x, midY + y, color);
      if (x != y) drawPixel(midX - y, midY + x, color);
      break;
    case 3:
      drawPixel(midX + x, midY + y, color);
      if (x != y) drawPixel(midX + y, midY + x, color);
      break;
    default:
      break;
    }
  }
}

void I2C_ssd1306::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
  x0 = (x0 < _width) ? x0 : (_width - 1);
  y0 = (y0 < _height) ? y0 : (_height - 1);
  x1 = (x1 < _width) ? x1 : (_width - 1);
  y1 = (y1 < _height) ? y1 : (_height - 1);
  int16_t slopeDirection = 1;
  bool swapped = false;

  if (y1 == y0){
    drawHLine(x0, y0, x1, color);
    return;
  }
  else if (x0 == x1){
    drawVLine(x0, y0, y1, color);
    return;
  }
  else if (y1 < y0) slopeDirection = -1;

  if (abs(x1 - x0) < abs(y1 - y0)) {
    swapped = true;
    _swap_uint8_t(x0, y0);
    _swap_uint8_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_uint8_t(x0, x1);
    _swap_uint8_t(y0, y1);
  }

  uint8_t dx = x1 - x0;
  uint8_t dy = abs(y1 - y0);

  int16_t err = dx >> 1;

  for (; x0 <= x1; x0++) {
    drawPixel(swapped ? y0 : x0, swapped ? x0 : y0, color);
    err -= dy;
    if (err < 0) {
      err += dx;
      y0 += slopeDirection;
    }
  }
}

//faster implementation of drawHLine without drawPixel call
void I2C_ssd1306::drawHLine(int16_t x0, int16_t y0, int16_t x1, uint8_t color) {
  y0 = (y0 < _height) ? y0 : (_height - 1);
  x0 = (x0 < _width) ? x0 : (_width - 1);
  x1 = (x1 < _width) ? x1 : (_width - 1);
  y0 = (y0 >= 0) ? y0 : (0);
  x0 = (x0 >= 0) ? x0 : (0);
  x1 = (x1 >= 0) ? x1 : (0);

  if (x0 > x1) _swap_int16_t(x0, x1);
  
  switch (color) {
      case COLOR_BLACK:
        for (; x0 <= x1; x0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] &= ~(1 << (y0 & 0b111));
        }
        break;
      case COLOR_WHITE:
        for (; x0 <= x1; x0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] |= (1 << (y0 & 0b111));
        }
        break;
      default:
        for (; x0 <= x1; x0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] ^= (1 << (y0 & 0b111));
        }
        break;
    }
}

/*
void I2C_ssd1306::drawHLine(int16_t x0, int16_t y0, int16_t x1, uint8_t color) {
  if (x0 > x1) _swap_int16_t(x0, x1);
  
  for (; x0 <= x1; x0++) {
    drawPixel(x0, y0, color);
  }
}*/

//faster implementation of drawVLine without drawPixel call
void I2C_ssd1306::drawVLine(int16_t x0, int16_t y0, int16_t y1, uint8_t color) {
  if (y0 > y1) _swap_int16_t(y0, y1);
  y0 = (y0 < _height) ? y0 : (_height - 1);
  x0 = (x0 < _width) ? x0 : (_width - 1);
  y1 = (y1 < _height) ? y1 : (_height - 1);
  y0 = (y0 >= 0) ? y0 : (0);
  x0 = (x0 >= 0) ? x0 : (0);
  y1 = (y1 >= 0) ? y1 : (0);
  
  switch (color) {
      case COLOR_BLACK:
        for (; y0 <= y1; y0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] &= ~(1 << (y0 & 0b111));
        }
        break;
      case COLOR_WHITE:
        for (; y0 <= y1; y0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] |= (1 << (y0 & 0b111));
        }
        break;
      default:
        for (; y0 <= y1; y0++) {
          _screenBuffer[((int)((y0 >> 3) * (_width)) + x0)] ^= (1 << (y0 & 0b111));
        }
        break;
    }
}

/*
void I2C_ssd1306::drawVLine(int16_t x0, int16_t y0, int16_t y1, uint8_t color) {
  if (y0 > y1) _swap_int16_t(y0, y1);
  
  for (; y0 <= y1; y0++) {
    drawPixel(x0, y0, color);
  }
}*/

void I2C_ssd1306::drawXBM(const uint8_t *bitmap, uint8_t height, uint8_t width, uint8_t x0, uint8_t y0, uint8_t color){
  height = height + y0 <= _height ?  height : _height - y0;
  uint8_t widthInBytes = (width+7) >> 3, bmpByte, dataBits;
  uint16_t totalBytes = (uint16_t)(widthInBytes * height);
  for(uint8_t y = 0; y < height; y++){
    for(uint8_t xByte = 0; xByte < widthInBytes; xByte++){
      bmpByte = pgm_read_byte(&bitmap[y * widthInBytes + xByte]);
      dataBits = ((xByte + 1) >> 3) > width ? width & 0b111 : 7;
      for(uint8_t i = 0; i <= dataBits; i++)
        if((bmpByte >> i) & 1) drawPixel(((xByte << 3) + i), y0 + y, color);
    }
  }
}

//http://ww1.microchip.com/downloads/en/AppNotes/01182b.pdf
void I2C_ssd1306::setFont(const unsigned char *fonts){
  _fontFamily = fonts;
  curFont.firstCharIndex = pgm_read_byte(&_fontFamily[0x03]) << 8 | pgm_read_byte_near(&_fontFamily[0x02]);
  curFont.lastCharindex = pgm_read_byte(&_fontFamily[0x05]) << 8 | pgm_read_byte_near(&_fontFamily[0x04]);
  curFont.charHeight = pgm_read_byte(&_fontFamily[0x06]);
  curFont.totalChars = (curFont.lastCharindex - curFont.firstCharIndex + 1);
}

void I2C_ssd1306::drawText(const unsigned char text[], uint8_t color){
  uint8_t charWidth, charBitmapByte, bitsLeft;
  uint16_t charHeadIndex;
  uint32_t charOffset;
  for(uint16_t i = 0; i < strlen((char *)text); i++){
    if(text[i] < curFont.firstCharIndex || text[i] > curFont.lastCharindex) continue;
    charHeadIndex =  (((int)text[i] - curFont.firstCharIndex) << 2) + 8 ;
    charWidth = (pgm_read_byte(&_fontFamily[charHeadIndex]));
    charOffset = ((uint32_t)pgm_read_byte(&_fontFamily[charHeadIndex + 3]) << 16) | (pgm_read_byte(&_fontFamily[charHeadIndex + 2]) << 8) | pgm_read_byte(&_fontFamily[charHeadIndex+1]);
    for(uint8_t clmnByte = 0; clmnByte < ((charWidth + 7) >> 3); clmnByte++){
      for(uint8_t y = 0; y < curFont.charHeight; y++){
        
        charBitmapByte = pgm_read_byte(&_fontFamily[charOffset + y * ((charWidth + 7) >> 3) + clmnByte]);
        bitsLeft = charWidth < (1 + clmnByte) << 3 ? (charWidth & 0b111) : 8;
        for(uint8_t x = 0; x < bitsLeft; x++){
          if((charBitmapByte >> x) & 1) drawPixel(_cursorX + (clmnByte << 3) + x, _cursorY + y, color);
        }
      }
    }
    _cursorX += charWidth + 1;
  }
}

void I2C_ssd1306::setDisplayOn(bool displayOn){
  if(displayOn) sendCommand(COMMAND_DISPLAY_ON);
  else sendCommand(COMMAND_DISPLAY_OFF);
}

void I2C_ssd1306::invertDisplay(bool invert){
  if(invert) sendCommand(COMMAND_SET_DISPLAY_INVERSE);
  else sendCommand(COMMAND_SET_DISPLAY_NORMAL);
}

void I2C_ssd1306::setContrast(uint8_t contrastValue) {
  START_TRANSMISSION
  wire->write(COMMAND_CONTRAST);
  wire->write(contrastValue);
  END_TRANSMISSION
}

void I2C_ssd1306::sendCommand(uint8_t command) {
  START_TRANSMISSION
  wire->write(commandByte);
  wire->write(command);
  END_TRANSMISSION
}

void I2C_ssd1306::sendCommandList(uint8_t* c_ptr, uint8_t listSize) {
  uint8_t bytesSent = 1;
  START_TRANSMISSION
  wire->write(commandByte);
  while (listSize--) {
    if (bytesSent >= MAX_I2C_BYTES) {
      END_TRANSMISSION
      START_TRANSMISSION
      wire->write(commandByte);
      bytesSent = 1;
    }
    wire->write((*c_ptr++));
    bytesSent++;
  }
  END_TRANSMISSION
}

void I2C_ssd1306::initialize() {
  uint8_t comPinsConf = 0x02;
  if(_width == 128 && _height == 64) comPinsConf = 0x12;
  uint8_t initList[] = {
    COMMAND_DISPLAY_OFF,
    COMMAND_MUX_RATIO,
    (_height - 1),
    COMMAND_SET_PAGE_ADDRESS,
    0, (_height / 8 - 1),
    COMMAND_SET_COLUMN_ADDRESS,
    0, (_width - 1),
    COMMAND_DISPLAY_OFFSET,
    (0x00),
    (0x40), //set display start line to 0
    COMMAND_SET_SEGMENT_RE_MAP_NORMAL,
    COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL,
    COMMAND_COM_PINS_CONFIGURATION,
    comPinsConf,
    COMMAND_MEMORY_ADDRESSING_MODE,
    0x00,
    COMMAND_CONTRAST,
    0xF7,
    COMMAND_DISABLE_ENTIRE_DISPLAY_ON,
    COMMAND_SET_DISPLAY_NORMAL,
    COMMAND_SET_CLOCK_DIV,
    0x80,
    COMMAND_CHARGE_PUMP,
    0x14,
    COMMAND_PRE_CHARGE,
    0x22,
    COMMAND_DEACTIVATE_SCROLL,
    COMMAND_DISPLAY_ON
  };
  sendCommandList(&initList[0], sizeof(initList));
  //free(initList);
  END_TRANSMISSION
  clearDisplay();
  display();
}

void I2C_ssd1306::_swap_uint8_t(uint8_t &a, uint8_t &b) {
  uint8_t temp = a;
  a = b;
  b = temp;
}

void I2C_ssd1306::_swap_int16_t(int16_t &a, int16_t &b) {
  a += b;
  b = a - b;
  a -= b;
}
