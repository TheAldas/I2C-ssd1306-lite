#include <Wire.h>
#ifndef I2C_ssd1306_h
#define I2C_ssd1306_h

#include "Arduino.h"
#include "Print.h"

#define COMMAND_DISPLAY_OFF 0xAE
#define COMMAND_DISPLAY_ON 0xAF
#define COMMAND_MUX_RATIO 0xA8
#define COMMAND_DISPLAY_OFFSET 0xD3
#define COMMAND_SET_SEGMENT_RE_MAP 0xA0
#define COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL 0xC0
#define COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_INVERSE 0xC8
#define COMMAND_COM_PINS_CONFIGURATION 0xDA //Set COM Pins hardware configuration
#define COMMAND_MEMORY_ADDRESSING_MODE 0x20//set addressing mode
#define COMMAND_CONTRAST 0x81//contrast between 0 and 255
#define COMMAND_DISABLE_ENTIRE_DISPLAY_ON 0xA4
#define COMMAND_ENABLE_ENTIRE_DISPLAY_ON 0xA5
#define COMMAND_SET_DISPLAY_NORMAL 0xA6
#define COMMAND_SET_DISPLAY_INVERSE 0xA7
#define COMMAND_SET_CLOCK_DIV 0xD5
#define COMMAND_CHARGE_PUMP 0x8D
#define COMMAND_PRE_CHARGE 0xD9 //set pre charge
#define COMMAND_DEACTIVATE_SCROLL 0x2E
#define COMMAND_SET_COLUMN_ADDRESS 0x21
#define COMMAND_SET_PAGE_ADDRESS 0x22

#define DISPLAY_FLIP_HORIZONTALLY 0x1

#define COLOR_BLACK 0
#define COLOR_WHITE 1
#define COLOR_INVERSE 2

#define START_TRANSMISSION wire->beginTransmission(_addr);
#define END_TRANSMISSION wire->endTransmission();
#define commandByte 0x00
#define dataByte 0x40
#define MAX_I2C_BYTES 32

#define ROUND(x) ((int)(x+0.5f))

class I2C_ssd1306:public Print {
  public:
    using Print::write;
    virtual size_t write(uint8_t c);

    I2C_ssd1306(uint8_t width, uint8_t height, byte ssd1306_address);
    void begin(TwoWire &I2Cwire);
    void display();
    void clearDisplay();
    void drawPixel(int16_t x0, int16_t y0, uint8_t color);
    void fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
    void fillRectRound(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t cornerRadius, uint8_t color);
    void fillCircle(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t color);
    void fillCircleQuarter(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t quarter, uint8_t color);
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
    void drawRectRound(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t cornerRadius, uint8_t color);
    void drawCircle(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t color);
    void drawCircleQuarter(uint8_t midX, uint8_t midY, uint8_t radius, uint8_t quarter, uint8_t color);
    void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
    void drawHLine(int16_t x0, int16_t y0, int16_t x1, uint8_t color);
    void drawVLine(int16_t x0, int16_t y0, int16_t y1, uint8_t color);
    void drawXBM(const uint8_t bitmap[], uint8_t height, uint8_t width, uint8_t x, uint8_t y, uint8_t color);
    void setFont(const unsigned char *fonts);
    void drawText(const char text[], uint8_t color);
    void setCursor(uint8_t column, uint8_t row);
    void setCursorCoord(uint8_t coordX, uint8_t coordY);
    void setCursorColumn(uint8_t column);
    void setCursorRow(uint8_t row);
    void advanceCursorRow(uint8_t rowCount, uint8_t column);
    void setDisplayOn(bool displayOn);
    void invertDisplay(bool invert);
    void flipVertically (bool flip);
    void setContrast(uint8_t contrastValue);
    
  private:
    void initialize();
    void sendCommand(uint8_t command);
    void sendCommandList(uint8_t *c_ptr, uint8_t listSize);
    void _swap_uint8_t(uint8_t &a, uint8_t &b);
    void _swap_int16_t(int16_t &a, int16_t &b);
    struct fontSummary
    {
      uint8_t charHeight;
      uint16_t firstCharIndex, lastCharindex, totalChars;
    } curFont;
    struct textConfiguration
    {
      int8_t lineSpacing = 1;
      int8_t letterSpacing = 1;
      uint8_t textColor = COLOR_WHITE;
    } textConf;
    
    const unsigned char *_fontFamily;
    uint8_t _fontSizeX = 5;
    uint8_t _fontSizeY = 7;
    uint8_t _cursorX = 0;
    uint8_t _cursorY = 0;
    TwoWire *wire;
    uint8_t _width, _height;
    byte _addr;
    uint8_t *_screenBuffer;
};
#endif
