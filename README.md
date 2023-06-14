# I2C ssd1306 lite
 A lighter library for oled screens with ssd1306
 As of may 23rd, 2023 the library is working correctly with Arduino UNO R4, no changes needed. The library on Uno R4 uses more flash memory
 
### Compatibility
 Currently only tested with atmega328p. Works with 128x32 and 128x64 oled screens.
 
### Library resource usage
* **128x64 Demo usage**
  * <12KB of Flash
  * <1500B of SRAM

* **128x32 Demo usage**
  * <11.7KB of Flash
  * <996B of SRAM

* **Empty, only initialized 128x32 without fonts**
  * <4.6KB of Flash

*  **Empty, only initialized 128x32 with picopixel fonts initialized and integer printed**
   * <6KB of Flash
   * <934B of SRAM

*  **Empty, only initialized 128x64 with picopixel fonts initialized and integer printed**
   * <6KB of Flash
   * <1446B of SRAM


### Current state
 Working on optimization. Currently the library is being perfected, because it lacks optimization to use less space, comments in the .h and .cpp files of the library, also it lacks documentation. Although, the library is useable and works at its current state.
