#include <Arduino.h>
#include "esp_camera.h"

#include "FS.h"               // SD Card ESP32
#include "SD_MMC.h"           // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

#include <filtering.h>

#include "img_converters.h"

#include "driver/rtc_io.h"

// Pin definition for AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);

  // camera pin configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[error] Camera init failed with error 0x%x", err);
    return;
  }

  // Init SD Card
  if(!SD_MMC.begin()) {
    Serial.println("[error] SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("[error] No SD Card attached");
    return;
  }
  // Sensor settings
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF); // Drop down frame size

  // Get frame buffer from esp32 camera
  camera_fb_t* fb = esp_camera_fb_get();

  if(!fb) {
    Serial.println("[error] Camera capture failed");
    return;
  }
  
  // convert frame buffer to matrix
  Matrix<PIXELFORMAT_RGB> mat(fb);
  
  Matrix<uint8_t> grayscale = filtering::convertToGrayscale(mat);

  // Image filtering
  Matrix<uint8_t> filtered = filtering::errorDiffusion(grayscale);

  fs::FS &fs = SD_MMC;

  // save from fb
  {
    
    File file = fs.open("/fb.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode.");
    }
    else {
      file.write(fb->buf, fb->len);
      Serial.printf("[error] File from fb Saved.");
    }
    file.close();
  }
  
  // Release camera FrameBuffer
  esp_camera_fb_return(fb); 

  {
    // Jpg conversion
    uint8_t* jpgBuffer = NULL;
    size_t jpgBufferLen = 0;
    fmt2jpg(grayscale.data(), grayscale.len(), grayscale.width(), grayscale.height(), PIXFORMAT_GRAYSCALE, 10, &jpgBuffer, &jpgBufferLen);
    
    File file = fs.open("/grayscale.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode");
    } 
    else {
      file.write(jpgBuffer, jpgBufferLen);
      Serial.printf("[error] File Saved.");
    }
    file.close();
  }

  {
    // Jpg conversion
    uint8_t* jpgBuffer = NULL;
    size_t jpgBufferLen = 0;
    fmt2jpg(filtered.data(), filtered.len(), filtered.width(), filtered.height(), PIXFORMAT_GRAYSCALE, 10, &jpgBuffer, &jpgBufferLen);
    
    File file = fs.open("/filtered.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode");
    } 
    else {
      file.write(jpgBuffer, jpgBufferLen);
      Serial.printf("[error] File Saved.");
    }
    file.close();
  }

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  // hold pin stage for deep sleep mode
  rtc_gpio_hold_en(GPIO_NUM_4);
  
  delay(2000);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}