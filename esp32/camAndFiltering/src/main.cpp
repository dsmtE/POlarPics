#include <Arduino.h>
#include "esp_camera.h"

#include "FS.h"               // SD Card ESP32
#include "SD_MMC.h"           // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

#include <cstdlib>

#include <filtering.h>

#include "img_converters.h"

#include "driver/rtc_io.h" // allows lock GPIO state  during sleep

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

void logPSRAM() {
  const uint32_t psramSize = ESP.getPsramSize();
  const uint32_t freePsram = ESP.getFreePsram();
  log_d("Psram -- Total: %d; Free: %d; Used: %d", psramSize, freePsram, psramSize - freePsram);
}

void logHeap() {

  const uint32_t heapSize = ESP.getHeapSize();
  const uint32_t freeHeap = ESP.getFreeHeap();
  log_d("Heap -- Total: %d; Free: %d; Used: %d", heapSize, freeHeap, heapSize - freeHeap);
}

void logMemory() {
  log_d("----- Memory log -----");
  logHeap();
  logPSRAM();
}

void fbToMat(camera_fb_t* fb, Matrix<PIXELFORMAT_RGB>& mat) {
    if (fb->width != mat.width() || fb->height != mat.height())
      throw std::runtime_error("[Error] Invalid size");

    // Load img and store it into our buffer
    if(!fmt2rgb888(fb->buf, fb->len, fb->format, (uint8_t*)mat.begin())) 
      throw std::runtime_error("[Error] getImageMatrixFromJPEGBuffer: conversion to rgb888 failed.");
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);

  // Enable flash
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  delay(2000);
  {
    // camera pin configuration
    const camera_config_t config = {
      PWDN_GPIO_NUM, // pin_pwdn
      RESET_GPIO_NUM, // pin_reset
      XCLK_GPIO_NUM, // pin_xclk
      SIOD_GPIO_NUM, // pin_sscb_sda
      SIOC_GPIO_NUM, // pin_sscb_scl
      Y9_GPIO_NUM, // pin_d7
      Y8_GPIO_NUM, // pin_d6
      Y7_GPIO_NUM, // pin_d5
      Y6_GPIO_NUM, // pin_d4
      Y5_GPIO_NUM, // pin_d3
      Y4_GPIO_NUM, // pin_d2
      Y3_GPIO_NUM, // pin_d1
      Y2_GPIO_NUM, // pin_d0
      VSYNC_GPIO_NUM, // pin_vsync
      HREF_GPIO_NUM, // pin_href
      PCLK_GPIO_NUM, // pin_pclk
      20000000, // xclk_freq_hz
      LEDC_TIMER_0, // ledc_timer
      LEDC_CHANNEL_0, // ledc_channel
      PIXFORMAT_JPEG, // pixel_format // PIXFORMAT_ + YUV422|GRAYSCALE|RGB565|JPEG
      // Init with high specs to pre-allocate larger buffers 
      psramFound() ? FRAMESIZE_UXGA : FRAMESIZE_SVGA, // frame_size // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
      psramFound() ? 10 : 12, // jpeg_quality - 0-63 lower number means higher quality.
      psramFound() ? 2 : 1, // fb_count - umber of frame buffers to be allocated.
    };

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      log_d("[error] Camera init failed with error 0x%x", err);
      return;
    }

    // Sensor settings
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_VGA); // Drop down frame size
    // s->set_quality(s, 1);
    // s->set_brightness(s, -2);
  }

  {
    // Init SD Card
    if(!SD_MMC.begin()) {
      log_d("[error] SD Card Mount Failed");
      return;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE) {
      log_d("[error] No SD Card attached");
      return;
    }
  }

  // Get frame buffer from esp32 camera
  camera_fb_t* fb = esp_camera_fb_get();

  if(!fb) {
    log_d("[error] Camera capture failed");
    return;
  }

  // convert frame buffer to matrix
  Matrix<PIXELFORMAT_RGB> mat(fb->width, fb->height);
  fbToMat(fb, mat);

  fs::FS &fs = SD_MMC;
  
  // save from fb
  {
    
    File file = fs.open("/fb.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode.");
    }
    else {
      file.write(fb->buf, fb->len);
      Serial.println("File from fb Saved.");
    }
    file.close();
  }
  
  // Release camera FrameBuffer
  esp_camera_fb_return(fb);

  {
    // Jpg conversion
    uint8_t* jpgBuffer = NULL;
    size_t jpgBufferLen = 0;
    fmt2jpg((uint8_t*)mat.begin(), mat.len() * 3, mat.width(), mat.height(), PIXFORMAT_RGB888, 100, &jpgBuffer, &jpgBufferLen);
    
    File file = fs.open("/mat.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode");
    } 
    else {
      file.write(jpgBuffer, jpgBufferLen);
      Serial.println("File from mat Saved.");
    }
    file.close();
  }

  // Image filtering
  Matrix<uint8_t> grayscale = filtering::convertToGrayscale(mat);

  {
    // Jpg conversion
    uint8_t* jpgBuffer = NULL;
    size_t jpgBufferLen = 0;
    fmt2jpg(grayscale.data(), grayscale.len(), grayscale.width(), grayscale.height(), PIXFORMAT_GRAYSCALE, 100, &jpgBuffer, &jpgBufferLen);
    
    File file = fs.open("/grayscale.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode");
    } 
    else {
      file.write(jpgBuffer, jpgBufferLen);
      Serial.println("File in grayscale Saved.");
    }
    file.close();
  }

  filtering::errorDiffusion(grayscale);
  Matrix<uint8_t> fitered(std::move(grayscale));

  log_d("grayscale pointer value : %s", grayscale.data() == NULL ? "NULL" : "not NULL");

    {
    // Jpg conversion
    uint8_t* jpgBuffer = NULL;
    size_t jpgBufferLen = 0;
    fmt2jpg(fitered.data(), fitered.len(), fitered.width(), fitered.height(), PIXFORMAT_GRAYSCALE, 100, &jpgBuffer, &jpgBufferLen);
    
    File file = fs.open("/fitered.jpg", FILE_WRITE);
    if(!file) {
      Serial.println("[error] Failed to open file in writing mode");
    } 
    else {
      file.write(jpgBuffer, jpgBufferLen);
      Serial.println("File in fitered Saved.");
    }
    file.close();
  }

  // Turns on the ESP32-CAM flash (connected to GPIO 4)
  digitalWrite(4, LOW);
  // hold pin stage for deep sleep mode
  rtc_gpio_hold_en(GPIO_NUM_4);
  
  Serial.println("Going to sleep now");
  delay(3000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}