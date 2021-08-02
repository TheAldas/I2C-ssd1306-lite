#include "I2C_ssd1306.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

I2C_ssd1306::I2C_ssd1306(uint8_t width, uint8_t height, byte ssd1306_address) {
  _width = width;
  _height = height;
  _addr = ssd1306_address;
  _screenBuffer = (uint8_t *)malloc((width * (height + 7) / 8));
}

I2C_ssd1306_minimal::I2C_ssd1306_minimal(uint8_t width, uint8_t height, byte ssd1306_address){
  _width = width;
  _height = height;
  _addr = ssd1306_address;
  _screenBuffer = (uint8_t *)malloc(width);
  _endX = 0;
  _startX = _width - 1;
}

void I2C_ssd1306::begin(TwoWire &I2Cwire) {
  wire = &I2Cwire;
  initialize();
}

void I2C_ssd1306_minimal::display(){
  if(_endX < _startX) return;
  uint8_t addrResList[] = {
    SSD_COMMAND_SET_PAGE_ADDRESS,
    _currentPage, _height - 1,
    SSD_COMMAND_SET_COLUMN_ADDRESS,
    _startX, (_width - 1)
  };
  sendCommandList(addrResList, sizeof(addrResList));
  #if defined(ESP8266)
  yield();
  #endif
  uint8_t columnsCount = (_endX - _startX) + 1;
  uint8_t bytesSent = 1;
  uint8_t *ptr = _screenBuffer + _startX;
  START_TRANSMISSION
  wire->write(SSD_dataByte);
  while (columnsCount--) {
    if (bytesSent >= MAX_I2C_BYTES) {
      END_TRANSMISSION
      START_TRANSMISSION
      wire->write(SSD_dataByte);
      bytesSent = 1;
    }
    wire->write(*ptr++);
    bytesSent++;
  }
  END_TRANSMISSION
  _endX = 0;
  _startX = _width - 1;
}

void I2C_ssd1306::display() {
  uint8_t addrResList[] = {
    SSD_COMMAND_SET_PAGE_ADDRESS,
    0x00, _height - 1,
    SSD_COMMAND_SET_COLUMN_ADDRESS,
    0x00, (_width - 1)
  };
  
  sendCommandList(addrResList, sizeof(addrResList));
  #if defined(ESP8266)
  yield();
  #endif
  uint16_t columnsCount = (_width * (_height + 7) / 8);
  uint8_t bytesSent = 1;
  uint8_t *ptr = _screenBuffer;
  START_TRANSMISSION
  wire->write(SSD_dataByte);
  while (columnsCount--) {
    if (bytesSent >= MAX_I2C_BYTES) {
      END_TRANSMISSION
      START_TRANSMISSION
      wire->write(SSD_dataByte);
      bytesSent = 1;
    }
    wire->write(*ptr++);
    bytesSent++;
  }
  END_TRANSMISSION
  #if defined(ESP8266)
  yield();
  #endif
}

void I2C_ssd1306::clearDisplay() {
  memset(_screenBuffer, 0, (_width * (_height + 7) / 8));
}

void I2C_ssd1306_minimal::clearDisplay() {
  clearPage();
  for(_currentPage = 0; _currentPage <= (_height + 7) >> 3; _currentPage++){
    _endX = _width - 1;
    _startX = 0;
    display();
  }
  _currentPage = 0;
  _endX = 0;
  _startX = _width - 1;
}

void I2C_ssd1306_minimal::clearPage(){
    memset(_screenBuffer, 0, (_width));
}

void I2C_ssd1306_minimal::drawPixel(int16_t x, int16_t y, uint8_t color) {
  if (x >= _width || y >= _height || x < 0 || y < 0) return;

  if((y / 8) != _currentPage){
    #if MINIMAL_AUTO
    display();
    clearPage();
    _currentPage = y / 8;
    _endX = 0;
    _startX = _width - 1;
    #else
      return;
    #endif
  }
  _endX = _endX < x ? x : _endX;
  _startX = _startX > x ? x : _startX;
  
  switch (color) {
    case SSD_COLOR_BLACK:
      _screenBuffer[x] &= ~(1 << (y & 0b111));
      break;
    case SSD_COLOR_WHITE:
      _screenBuffer[x] |= (1 << (y & 0b111));
      break;
    default:
      _screenBuffer[x] ^= (1 << (y & 0b111));
      break;
  }
}

void I2C_ssd1306::drawPixel(int16_t x, int16_t y, uint8_t color) {
  if (x >= _width || y >= _height || x < 0 || y < 0) return;

  switch (color) {
    case SSD_COLOR_BLACK:
      _screenBuffer[((int)((y >> 3) * (_width)) + x)] &= ~(1 << (y & 0b111));
      break;
    case SSD_COLOR_WHITE:
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

  if (abs(x1 - x0) < abs(y1 - y0)) {
    swapped = true;
    _swap_uint8_t(x0, y0);
    _swap_uint8_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_uint8_t(x0, x1);
    _swap_uint8_t(y0, y1);
  }

  if (y1 < y0) slopeDirection = -1;

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

void I2C_ssd1306::drawHLine(int16_t x0, int16_t y0, int16_t x1, uint8_t color) {
  y0 = (y0 < _height) ? y0 : (_height - 1);
  x0 = (x0 < _width) ? x0 : (_width - 1);
  x1 = (x1 < _width) ? x1 : (_width - 1);
  y0 = (y0 >= 0) ? y0 : (0);
  x0 = (x0 >= 0) ? x0 : (0);
  x1 = (x1 >= 0) ? x1 : (0);

  if (x0 > x1) _swap_int16_t(x0, x1);
  
  for (; x0 <= x1; x0++) {
    drawPixel(x0, y0, color);
  }
}

void I2C_ssd1306::drawVLine(int16_t x0, int16_t y0, int16_t y1, uint8_t color) {
  y0 = (y0 < _height) ? y0 : (_height - 1);
  x0 = (x0 < _width) ? x0 : (_width - 1);
  y1 = (y1 < _height) ? y1 : (_height - 1);
  y0 = (y0 >= 0) ? y0 : (0);
  x0 = (x0 >= 0) ? x0 : (0);
  y1 = (y1 >= 0) ? y1 : (0);

  if (y0 > y1) _swap_int16_t(y0, y1);
  
  for (; y0 <= y1; y0++) {
    drawPixel(x0, y0, color);
  }
}

void I2C_ssd1306::drawXBM(const uint8_t *bitmap, uint8_t width, uint8_t height, uint8_t x0, uint8_t y0, uint8_t color){
  height = height + y0 <= _height ?  height : _height - y0;
  uint8_t widthInBytes = (width+7) >> 3, bmpByte, dataBits;
  uint16_t totalBytes = (uint16_t)(widthInBytes * height);
  for(uint8_t y = 0; y < height; y++){
    for(uint8_t xByte = 0; xByte < widthInBytes; xByte++){
      bmpByte = pgm_read_byte(&bitmap[y * widthInBytes + xByte]);
      dataBits = ((xByte + 1) >> 3) > width ? width & 0b111 : 7;
      for(uint8_t i = 0; i <= dataBits; i++)
        if((bmpByte >> i) & 1) drawPixel((x0 + (xByte << 3) + i), y0 + y, color);
    }
  }
}

//http://ww1.microchip.com/downloads/en/AppNotes/01182b.pdf
void I2C_ssd1306::setFont(const unsigned char *fonts){
  _fontFamily = fonts;
  curFont.firstCharIndex = pgm_read_byte(&_fontFamily[0x03]) << 8 | pgm_read_byte_near(&_fontFamily[0x02]);
  curFont.lastCharIndex = pgm_read_byte(&_fontFamily[0x05]) << 8 | pgm_read_byte_near(&_fontFamily[0x04]);
  curFont.charHeight = pgm_read_byte(&_fontFamily[0x06]);
}

size_t I2C_ssd1306::write(uint8_t c){
  uint8_t charBitmapByte, bitsLeft;
  if(c == '\n'){ //transfer to new line
    _cursorX = 0;
    _cursorY += curFont.charHeight * textConf.textScale + textConf.lineSpacing;
    return 1;
  }
  if(c == '\r') return 1; //ignoring carriage return
  if(c < curFont.firstCharIndex || c > curFont.lastCharIndex) return 1;
  uint16_t charHeadIndex =  (((int)c - curFont.firstCharIndex) << 2) + 8 ;
  uint8_t charWidth = (pgm_read_byte(&_fontFamily[charHeadIndex]));
  uint32_t charOffset = (((uint32_t)pgm_read_byte(&_fontFamily[charHeadIndex + 3])) << 16) | (((uint16_t)pgm_read_byte(&_fontFamily[charHeadIndex + 2])) << 8) | pgm_read_byte(&_fontFamily[charHeadIndex+1]);
  for(uint8_t clmnByte = 0; clmnByte < ((charWidth + 7) >> 3); clmnByte++){
    for(uint8_t y = 0; y < curFont.charHeight; y++){
      
      charBitmapByte = pgm_read_byte(&_fontFamily[charOffset + y * ((charWidth + 7) >> 3) + clmnByte]);
      bitsLeft = charWidth < (1 + clmnByte) << 3 ? (charWidth & 0b111) : 8;
      for(uint8_t x = 0; x < bitsLeft; x++){
        if((charBitmapByte >> x) & 1)
          for(uint8_t sX = 0; sX < textConf.textScale; sX++)
            for(uint8_t sY = 0; sY < textConf.textScale; sY++)
              drawPixel(textConf.offsetX + _cursorX + (clmnByte << 3) + x * textConf.textScale + sX, textConf.offsetY + _cursorY + y * textConf.textScale + sY, textConf.textColor);
      }
    }
  }
  _cursorX += charWidth * textConf.textScale + textConf.letterSpacing;
  return 1;
}

void I2C_ssd1306::drawText(const char text[], uint8_t color){
  uint8_t charWidth, charBitmapByte, bitsLeft;
  uint16_t charHeadIndex;
  uint32_t charOffset;
  for(uint16_t i = 0; i < strlen((char *)text); i++){
    if(text[i] == '\n'){ //transfer to new line
      _cursorX = 0;
      _cursorY += curFont.charHeight * textConf.textScale + textConf.lineSpacing;
      continue;
    }
    if(text[i] < curFont.firstCharIndex || text[i] > curFont.lastCharIndex) continue;
    charHeadIndex =  (((int)text[i] - curFont.firstCharIndex) << 2) + 8 ;
    charWidth = (pgm_read_byte(&_fontFamily[charHeadIndex]));
    charOffset = ((uint32_t)pgm_read_byte(&_fontFamily[charHeadIndex + 3]) << 16) | (pgm_read_byte(&_fontFamily[charHeadIndex + 2]) << 8) | pgm_read_byte(&_fontFamily[charHeadIndex+1]);
    for(uint8_t clmnByte = 0; clmnByte < ((charWidth + 7) >> 3); clmnByte++){
      for(uint8_t y = 0; y < curFont.charHeight; y++){
        
        charBitmapByte = pgm_read_byte(&_fontFamily[charOffset + y * ((charWidth + 7) >> 3) + clmnByte]);
        bitsLeft = charWidth < (1 + clmnByte) << 3 ? (charWidth & 0b111) : 8;
        for(uint8_t x = 0; x < bitsLeft; x++){
        if((charBitmapByte >> x) & 1)
          for(uint8_t sX = 0; sX < textConf.textScale; sX++)
            for(uint8_t sY = 0; sY < textConf.textScale; sY++)
              drawPixel(textConf.offsetX + _cursorX + (clmnByte << 3) + x * textConf.textScale + sX, textConf.offsetY + _cursorY + y * textConf.textScale + sY, textConf.textColor);
        }
      }
    }
    _cursorX += charWidth * textConf.textScale + textConf.letterSpacing;
  }
}

uint16_t I2C_ssd1306::getTextWidth(const char text[]){
  uint8_t charWidth;
  uint16_t charHeadIndex, width = 0;
  for(uint16_t i = 0; i < strlen((char *)text); i++){
    if(text[i] < curFont.firstCharIndex || text[i] > curFont.lastCharIndex) continue;
    charHeadIndex =  (((int)text[i] - curFont.firstCharIndex) << 2) + 8 ;
    charWidth = (pgm_read_byte(&_fontFamily[charHeadIndex]));
    width += charWidth * textConf.textScale + textConf.letterSpacing;
  }
  return width;
}

void I2C_ssd1306::setCursor(uint8_t column, uint8_t row){
  _cursorX = column;
  _cursorY = (curFont.charHeight * textConf.textScale * row) + (textConf.lineSpacing * row);
}

void I2C_ssd1306::setCursorCoord(uint8_t coordX, uint8_t coordY){
  _cursorX = coordX;
  _cursorY = coordY;
}

void I2C_ssd1306::advanceCursorRow(uint8_t rowCount, uint8_t column){
  _cursorY += (curFont.charHeight * textConf.textScale * rowCount) + (textConf.lineSpacing * rowCount);
  _cursorX = column;
}

void I2C_ssd1306::setDisplayOn(bool displayOn){
  if(displayOn) sendCommand(SSD_COMMAND_DISPLAY_ON);
  else sendCommand(SSD_COMMAND_DISPLAY_OFF);
}

void I2C_ssd1306::invertDisplay(bool invert){
  if(invert) sendCommand(SSD_COMMAND_SET_DISPLAY_INVERSE);
  else sendCommand(SSD_COMMAND_SET_DISPLAY_NORMAL);
}

void I2C_ssd1306::flipVertically(bool flip){
  if(flip){
    sendCommand(SSD_COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_INVERSE);
  }else{
    sendCommand(SSD_COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL);
  }
}

void I2C_ssd1306::setContrast(uint8_t contrastValue) {
  START_TRANSMISSION
  wire->write(SSD_commandByte);
  wire->write(SSD_COMMAND_CONTRAST);
  wire->write(contrastValue);
  END_TRANSMISSION
}

void I2C_ssd1306::sendCommand(uint8_t command) {
  START_TRANSMISSION
  wire->write(SSD_commandByte);
  wire->write(command);
  END_TRANSMISSION
}

void I2C_ssd1306::sendCommandList(uint8_t* c_ptr, uint8_t listSize) {
  uint8_t bytesSent = 1;
  START_TRANSMISSION
  wire->write(SSD_commandByte);
  while (listSize--) {
    if (bytesSent >= MAX_I2C_BYTES) {
      END_TRANSMISSION
      START_TRANSMISSION
      wire->write(SSD_commandByte);
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
    SSD_COMMAND_DISPLAY_OFF,
    SSD_COMMAND_MUX_RATIO,
    (_height - 1),
    SSD_COMMAND_SET_PAGE_ADDRESS,
    0, (_height / 8 - 1),
    SSD_COMMAND_SET_COLUMN_ADDRESS,
    0, (_width - 1),
    SSD_COMMAND_DISPLAY_OFFSET,
    (0x00),
    (0x40), //set display start line to 0
    SSD_COMMAND_SET_SEGMENT_RE_MAP | SSD_DISPLAY_FLIP_HORIZONTALLY,
    SSD_COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_INVERSE,
    SSD_COMMAND_COM_PINS_CONFIGURATION,
    comPinsConf,
    SSD_COMMAND_MEMORY_ADDRESSING_MODE,
    0x00,
    SSD_COMMAND_CONTRAST,
    0xF7,
    SSD_COMMAND_DISABLE_ENTIRE_DISPLAY_ON,
    SSD_COMMAND_SET_DISPLAY_NORMAL,
    SSD_COMMAND_SET_CLOCK_DIV,
    0x80,
    SSD_COMMAND_CHARGE_PUMP,
    0x14,
    SSD_COMMAND_PRE_CHARGE,
    0x22,
    SSD_COMMAND_DEACTIVATE_SCROLL,
    SSD_COMMAND_DISPLAY_ON
  };
  sendCommandList(&initList[0], sizeof(initList));
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
