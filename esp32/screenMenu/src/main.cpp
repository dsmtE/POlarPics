#include <Arduino.h> // Core

#include "esp_camera.h" // Cam

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

#include <filtering.h>

#include "img_converters.h"

#include "driver/rtc_io.h" // allows lock GPIO state  during sleep

#include "esp32-hal.h" // used to allow ps_malloc

#include <cstdlib>
#include <stdexcept>
#include <string.h>
#include <array>
#include <string>
#include <vector>
#include <algorithm>

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

void fbToMat(camera_fb_t* fb, Matrix<PIXELFORMAT_RGB>& mat) {
    if (fb->width != mat.width() || fb->height != mat.height())
      throw std::runtime_error("[Error] Invalid size");

    // Load img and store it into our buffer
    if(!fmt2rgb888(fb->buf, fb->len, fb->format, (uint8_t*)mat.begin())) 
      throw std::runtime_error("[Error] getImageMatrixFromJPEGBuffer: conversion to rgb888 failed.");
}

uint16_t colorConverter(const uint8_t r,const uint8_t g, const uint8_t b) {
    // R, G, and B are divided equally giving 3 * 5 = 15 bits and the additional bit is allocated to Green.
    // (565 rgb format)
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
uint16_t colorConverter(const uint8_t grey) {
    return colorConverter(grey, grey, grey);
}

void drawGrayScale(TFT_eSPI& tft, int16_t x, int16_t y, const Matrix<uint8_t>& grayscaleMat) {
    for (int16_t r = 0; r < grayscaleMat.height(); ++r) {
        for (int16_t c = 0; c < grayscaleMat.width(); ++c) {
            const uint8_t grey = grayscaleMat(r, c);
            tft.drawPixel(x + c, y + r, tft.color565(grey, grey, grey));
            // 320*240
        }
    }
}

template <typename T>
const Matrix<T> rescale(const Matrix<T>& in, const int8_t ratioNum, const int8_t ratioDenom) {
    const size_t newWidth = in.width() * ratioNum / ratioDenom;
    const size_t newHeight = in.height() * ratioNum / ratioDenom;
    Matrix<T> out(newWidth, newHeight);
    for (size_t r = 0; r < newHeight; ++r) {
        for (size_t c = 0; c < newWidth; ++c) {
            std::vector<T> values(ratioDenom*ratioDenom);
            // compute subKernel input values
            for (size_t j = 0; j < ratioDenom; ++j) {
                for (size_t i = 0; i < ratioDenom; ++i) {
                    values[i + j * ratioDenom] = in((r * ratioDenom + i) / ratioNum, (c* ratioDenom + j) / ratioNum);
                }
            }
            // here simply take the average of input values instead of apply a specific kernel
            T sum = std::accumulate(values.begin(), values.end(), T(0));
            out(r, c) = sum / ratioDenom*ratioDenom;
        }
    }
    return out;
}

template <typename T>
const Matrix<T> subMatrix(const Matrix<T>& in, const size_t x, const size_t y, const size_t w, const size_t h) {
    assert(x+w < in.width() && x+h < in.width());
    Matrix<T> out(w, h);
    for (size_t r = 0; r < h; ++r) {
        for (size_t c = 0; c < w; ++c) {
            out(r, c) = in(y + r, x + c);
        }
    }
    return out;
}

Matrix<uint8_t> grayscale;

char* ToBitmap (const Matrix<uint8_t>& grayscale) {
    const int bitmapLen = std::ceil(grayscale.len() / 8);
    char* bitmap = (char*) ps_malloc(bitmapLen);
    
    // TODO
    return bitmap;
}

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 500

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
// 240*320

void setup(void) {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    Serial.begin(115200);

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
      psramFound() ? size_t(2): size_t(1) // fb_count - umber of frame buffers to be allocated.
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
    // 640x480
  }

    tft.init();
    tft.setRotation(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(1);
    tft.setTextSize(1);
}

void loop() {
    
    // convert frame buffer to matrix
    {
        camera_fb_t* fb = esp_camera_fb_get();
        Matrix<PIXELFORMAT_RGB> mat(fb->width, fb->height);
        fbToMat(fb, mat);
        esp_camera_fb_return(fb);
        grayscale = filtering::convertToGrayscale(mat);

        {
            char strBuffer[100];
            sprintf(strBuffer, "matrix size (w,h): ( %d, %d)\n", grayscale.width(), grayscale.height());
            Serial.println(strBuffer);
        }

        grayscale = std::move(rescale(grayscale, 1, 2));

        {
            char strBuffer[100];
            sprintf(strBuffer, "matrix size after rescale (w,h): ( %d, %d)\n", grayscale.width(), grayscale.height());
            Serial.println(strBuffer);
        }

/*
        Serial.println("display img :");
        for (size_t r = 0; r < 100; ++r) {
            for (size_t c = 0; c < 100; ++c) {
                Serial.print(grayscale(r, c));
                Serial.print(", ");
            }
            Serial.println(' ');
        }
        */
        // filtering::errorDiffusion(grayscale);
    }
    // tft.fillScreen(TFT_WHITE);
    
    drawGrayScale(tft, 0, 0, grayscale);

    delay(WAIT);
}