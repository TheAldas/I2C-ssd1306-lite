#include <Wire.h>
#include <Arduino.h>
#include <I2C_ssd1306.h> //I2C SSD1306 lite library
#include <Fonts/Picopixel5x6.h> //Fonts that will be used
#include <splash128x32.h> //splash screen image file. Image is in XBM format


#define SCREEN_WIDTH 128 //Width of the screen
#define SCREEN_HEIGHT 32 //Height of the screen

#define OLED_ADDRESS 0x3C //address of the screen

I2C_ssd1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_ADDRESS);

void setupDisplayFont();

void demoStart();

void demoDrawLine();
void demoDrawRect();
void demoDrawRectRound();
void demoDrawCircle();
void demoDrawCircleQuarter();

void demoFillRect();
void demoFillRectRound();
void demoFillCircle();
void demoFillCircleQuarter();

void demoPrint();
void demoEnd();

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  oled.begin(Wire);
  setupDisplayFont();

  demoStart();
  demoDrawLine();
  demoDrawRect();
  demoDrawRectRound();
  demoDrawCircle();
  demoDrawCircleQuarter();
  
  demoFillRect();
  demoFillRectRound();
  demoFillCircle();
  demoFillCircleQuarter();
  
  demoPrint();
  demoEnd();
}


void loop() {
}

//sets fonts that will be used in demo
void setupDisplayFont(){
  oled.setFont(Picopixel5x6);
}

//draws centered splash screen image and fades in display
void demoStart(){
  oled.clearDisplay();

  //set screen brightness to 0
  oled.setContrast(0);

  //draw splash screen image
  oled.drawXBM(splash128x32_bits, splash128x32_width, splash128x32_height, 
              (oled.getWidth() - splash128x32_width) /2, (oled.getHeight() - splash128x32_height) / 2, SSD_COLOR_WHITE);
  oled.display();
  
  //fade in
  uint8_t i;
  for(i = 0; i < 0xAF; i+=5){
    oled.setContrast(i);
    delay(125);
  }
}

//draws lines 2 times going from (0;0) to (<screen width> - 1; <screen height> - 1)
void demoDrawLine(){
  for(uint8_t j = 0; j < 2; j++){
    for(uint8_t x = 0; x < oled.getWidth(); x++){
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.drawLine(x, 0, oled.getWidth() - x - 1, oled.getHeight() - 1, SSD_COLOR_WHITE);
      oled.display();
      delay(10);
    }
    for(uint8_t y = 0; y < oled.getHeight(); y++){
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.drawLine(0, oled.getHeight() - y - 1, oled.getWidth() - 1, y, SSD_COLOR_WHITE);
      oled.display();
      delay(20);
    }
  }
}

//draws rectangle with changing dimensions
void demoDrawRect(){
  // w -> width of rectangle| h -> height of rectangle
  uint8_t w, h = oled.getHeight() / 2;
  for(w = 2; w < oled.getWidth() / 2; w++){
    oled.clearDisplay();
    oled.drawRect((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, SSD_COLOR_WHITE); //draw centered rectangle
    oled.display();
    delay(20);
  }
  for(; h > 1; h--){
    oled.clearDisplay();
    oled.drawRect((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, SSD_COLOR_WHITE); //draw centered rectangle
    oled.display();
    delay(20);
  }
}

//draws rounded rectangle with changing corner radius
void demoDrawRectRound(){
  // w -> width of rectangle| h -> height of rectangle
  uint8_t w = oled.getWidth() / 2, h = oled.getHeight() / 2;
  for(uint8_t radius = 2; radius < h / 2; radius++){
    oled.clearDisplay();
    oled.drawRectRound((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, radius, SSD_COLOR_WHITE); //draw centered round rectangle
    oled.display();
    delay(20);
  }
}

//draws changing size circle
void demoDrawCircle(){
  //x0, y0 -> circle middle coordinates
  uint8_t radius, x0 = oled.getWidth() / 2, y0 = oled.getHeight() / 2;
  for(radius = 3; radius < y0 - 2; radius++){
    oled.clearDisplay();
    oled.drawCircle(x0, y0, radius, SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
}

//draws spinning circle quarter
void demoDrawCircleQuarter(){
  //x0, y0 -> circle middle coordinates
  uint8_t quarter, x0 = oled.getWidth() / 2, y0 = oled.getHeight() / 2;
  for(quarter = 0; quarter < 16; quarter++){
    oled.clearDisplay();
    oled.drawCircleQuarter(x0, y0, y0 / 2, quarter % 4, SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
  for(quarter = 16; quarter > 0; quarter--){
    oled.clearDisplay();
    oled.drawCircleQuarter(x0, y0, y0 / 2, (quarter % 4), SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
}

//draws filled rectangle with changing dimensions
void demoFillRect(){
  // w -> width of rectangle| h -> height of rectangle
  uint8_t w, h = oled.getHeight() / 2;
  for(w = 2; w < oled.getWidth() / 2; w++){
    oled.clearDisplay();
    oled.fillRect((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, SSD_COLOR_WHITE); //draw centered rectangle
    oled.display();
    delay(20);
  }
  for(; h > 1; h--){
    oled.clearDisplay();
    oled.fillRect((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, SSD_COLOR_WHITE); //draw centered rectangle
    oled.display();
    delay(20);
  }
}

//draws filled rounded rectangle with changing corner radius
void demoFillRectRound(){
  // w -> width of rectangle| h -> height of rectangle
  uint8_t w = oled.getWidth() / 2, h = oled.getHeight() / 2;
  for(uint8_t radius = 2; radius < min(h, w) / 2; radius++){
    oled.clearDisplay();
    oled.fillRectRound((oled.getWidth() - w) /2, (oled.getHeight() - h) / 2, w, h, radius, SSD_COLOR_WHITE);
    oled.display();
    delay(20);
  }
}

//draws filled circle with changing radius
void demoFillCircle(){
  // x0, y0 -> coordinates of the circle middle
  uint8_t radius, x0 = oled.getWidth() / 2, y0 = oled.getHeight() / 2;
  for(radius = 3; radius < y0 - 2; radius++){
    oled.clearDisplay();
    oled.fillCircle(x0, y0, radius, SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
}

//draws spinning filled circle quarter
void demoFillCircleQuarter(){
  // x0, y0 -> coordinates of the circle middle
  uint8_t quarter, x0 = oled.getWidth() / 2, y0 = oled.getHeight() / 2;
  for(quarter = 0; quarter < 16; quarter++){
    oled.clearDisplay();
    oled.fillCircleQuarter(x0, y0, y0 / 2, quarter % 4, SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
  for(quarter = 16; quarter > 0; quarter--){
    oled.clearDisplay();
    oled.fillCircleQuarter(x0, y0, y0 / 2, (quarter % 4), SSD_COLOR_WHITE);
    oled.display();
    delay(50);
  }
}

//draws text
void demoPrint(){
  const char text[] = "Hello world!";
  const char text2[] = "Centered text";

  oled.clearDisplay();

  //sets cursor to first column and first row
  oled.setCursor(0, 0);
  //prints text one character at a time
  for(uint16_t i = 0; i < strlen(text); i++){
    oled.print(text[i]);
    oled.display();
    delay(200);
  }

  //advance cursor by 1 row and set cursor column to around the middle of the screen to later print a text in the middle of the screen
  oled.advanceCursorRow(1, (oled.getWidth() - oled.getTextWidth(text2)) / 2);
  //prints centered text in the second row one character at a time
  for(uint16_t i = 0; i < strlen(text2); i++){
    oled.print(text2[i]);
    oled.display();
    delay(200);
  }

  oled.invertDisplay(true);
  delay(1000);
  oled.invertDisplay(false);
  delay(1000);
}

//endlessly draws a spinning circle quarter that shrinks or expands filled circle
void demoEnd(){
  oled.clearDisplay();
  uint8_t h = oled.getHeight(), w = oled.getWidth(), radius = h / 3, i;
  bool shrinking = true;
  //draws filled circle in the middle of the screen
  oled.fillCircle(w / 2, h / 2, (h / 3), SSD_COLOR_WHITE);
  //loop forever
  for(;;){
    for(i = 0; i < 4; i++){
      //draw circle quarter onto bigger filled circle
      oled.drawCircleQuarter(w / 2, h / 2, radius, i, SSD_COLOR_INVERSE);
      oled.display();
      delay(20);
    }
    if(shrinking) radius--;
    else radius++;

    shrinking = radius > 3 && radius <= h / 3 ? shrinking : !shrinking;
  }
}

