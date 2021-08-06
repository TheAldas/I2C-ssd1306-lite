#include <Wire.h>
#ifndef I2C_ssd1306_h
#define I2C_ssd1306_h

#include "Arduino.h"
#include "Print.h"

#define SSD_COMMAND_DISPLAY_OFF 0xAE
#define SSD_COMMAND_DISPLAY_ON 0xAF
#define SSD_COMMAND_MUX_RATIO 0xA8
#define SSD_COMMAND_DISPLAY_OFFSET 0xD3
#define SSD_COMMAND_SET_SEGMENT_RE_MAP 0xA0
#define SSD_COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL 0xC0
#define SSD_COMMAND_SET_COM_OUTPUT_SCAN_DIRECTION_INVERSE 0xC8
#define SSD_COMMAND_COM_PINS_CONFIGURATION 0xDA //Set COM Pins hardware configuration
#define SSD_COMMAND_MEMORY_ADDRESSING_MODE 0x20//set addressing mode
#define SSD_COMMAND_CONTRAST 0x81//contrast between 0 and 255
#define SSD_COMMAND_DISABLE_ENTIRE_DISPLAY_ON 0xA4
#define SSD_COMMAND_ENABLE_ENTIRE_DISPLAY_ON 0xA5
#define SSD_COMMAND_SET_DISPLAY_NORMAL 0xA6
#define SSD_COMMAND_SET_DISPLAY_INVERSE 0xA7
#define SSD_COMMAND_SET_CLOCK_DIV 0xD5
#define SSD_COMMAND_CHARGE_PUMP 0x8D
#define SSD_COMMAND_PRE_CHARGE 0xD9 //set pre charge
#define SSD_COMMAND_DEACTIVATE_SCROLL 0x2E
#define SSD_COMMAND_SET_COLUMN_ADDRESS 0x21
#define SSD_COMMAND_SET_PAGE_ADDRESS 0x22

#define SSD_DISPLAY_FLIP_HORIZONTALLY 0x1

/*
if the pixel is being drawn out of currently selected page bounds, it's not drawn
*/
#define SSD_MINIMAL_MODE_MANUAL 0 
/*
  if the pixel is being drawn out of currently selected page bounds,
  current page buffer is displayed, then cleared and pixel is drawn in bounds of the page.
  See minimal class drawPixel() function for better understanding.
*/
#define SSD_MINIMAL_MODE_AUTO 1 

#define SSD_COLOR_BLACK 0
#define SSD_COLOR_WHITE 1
#define SSD_COLOR_INVERSE 2

#define START_TRANSMISSION wire->beginTransmission(_addr);
#define END_TRANSMISSION wire->endTransmission();
#define SSD_commandByte 0x00
#define SSD_dataByte 0x40
#define MAX_I2C_BYTES 30

#define ROUND(x) ((int)(x+0.5f))

class I2C_ssd1306:public Print {
  public:
    using Print::write;
    virtual size_t write(uint8_t c);

    I2C_ssd1306(uint8_t width, uint8_t height, byte ssd1306_address);
    I2C_ssd1306(){}
    void begin(TwoWire &I2Cwire);
    virtual void display();
    virtual void clearDisplay();
    virtual void drawPixel(int16_t x0, int16_t y0, uint8_t color);
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
    void drawXBM(const uint8_t bitmap[], uint8_t width, uint8_t height, uint8_t x, uint8_t y, uint8_t color);
    void setFont(const unsigned char *fonts);
    uint8_t getFontHeight() { return curFont.charHeight * textConf.textScale; };
    void drawText(const char text[], uint8_t color);
    uint16_t getTextWidth(const char text[]);
    void setTextOffset(uint8_t offsetX, uint8_t offsetY) { textConf.offsetX = offsetX; textConf.offsetY = offsetY;};
    void setTextScale(uint8_t textScale) { textConf.textScale = textScale;};
    void setTextLineSpacing(uint8_t lineSpacing) { textConf.lineSpacing = lineSpacing; };
    void setTextLetterSpacing(uint8_t letterSpacing) { textConf.letterSpacing; };
    void setCursor(uint8_t column, uint8_t row);
    void setCursorCoord(uint8_t coordX, uint8_t coordY);
    void setCursorColumn(uint8_t column){_cursorX = column;}
    void setCursorRow(uint8_t row) {_cursorY = (curFont.charHeight * textConf.textScale * row) + (textConf.lineSpacing * row);}
    void advanceCursorRow(uint8_t rowCount, uint8_t column);
    void setDisplayOn(bool displayOn);
    void invertDisplay(bool invert);
    void flipVertically (bool flip);
    void setContrast(uint8_t contrastValue);
    uint8_t getHeight(){return _height;}
    uint8_t getWidth(){return _width;}
    
  protected:
    virtual void initialize();
    void sendCommand(uint8_t command);
    void sendCommandList(uint8_t *c_ptr, uint8_t listSize);
    void _swap_uint8_t(uint8_t &a, uint8_t &b);
    void _swap_int16_t(int16_t &a, int16_t &b);
    struct fontSummary
    {
      uint8_t charHeight;
      uint16_t firstCharIndex, lastCharIndex;
    } curFont;
    struct textConfiguration
    {
      int8_t lineSpacing = 2;
      int8_t letterSpacing = 1;
      uint8_t textColor = SSD_COLOR_WHITE;
      uint8_t textScale = 1;
      uint8_t offsetX = 0, offsetY = 0;
    } textConf;
    
    const unsigned char *_fontFamily;
    uint8_t _cursorX = 0;
    uint8_t _cursorY = 0;
    TwoWire *wire;
    uint8_t _width, _height;
    byte _addr;
    uint8_t *_screenBuffer;
};

class I2C_ssd1306_minimal : public I2C_ssd1306
{
  public:
    I2C_ssd1306_minimal(uint8_t width, uint8_t height, byte ssd1306_address);
    void clearPage();
    void display();
    void clearDisplay();
    void drawPixel(int16_t x0, int16_t y0, uint8_t color);
    void setPage(uint8_t page){ if(page < ((_height + 7) / 8)) {_currentPage = page; clearPage();}}
    uint8_t getPage() {return _currentPage;}
    void setMinimalMode(uint8_t mode) { _mode = mode;};
  private:
    uint8_t _currentPage = 0, _startX, _endX;
    uint8_t _mode = SSD_MINIMAL_MODE_AUTO;
};


#endif
