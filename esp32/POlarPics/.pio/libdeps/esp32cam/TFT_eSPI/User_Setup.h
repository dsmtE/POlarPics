// ###### SCREEN DRIVER   ######
#define ILI9341_DRIVER

// ###### SIZE   ######
#define TFT_WIDTH  240 // 240 x 320
#define TFT_HEIGHT 320

// ###### ESP32 PIN SETUP   ######
#define TFT_MOSI 13 // SDA
#define TFT_SCLK 14 // SCL
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST  16  // Reset pin

#define ST7735_GREENTAB2

#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters

#define SPI_FREQUENCY  27000000 // Actually sets it to 26.67MHz = 80/3
