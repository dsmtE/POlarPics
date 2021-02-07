#include <Arduino.h> // Core

#include "esp_camera.h" // Cam

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

#include "img_converters.h"

#include "driver/rtc_io.h" // allows lock GPIO state  during sleep

#include "esp32-hal.h" // used to allow ps_malloc

// #include <cstdlib>
#include <stdexcept>
#include <array>

#include "Matrix.h"
#include "PrinterMatrix.h"

#include "utils.hpp"
#include "menu.hpp"

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


#define buttonPin  0

sensor_t * s;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// matrix buffers
Matrix<uint8_t> grayscale;
PrinterMatrix ditherMat;

// menuVariables
size_t buttonsValue;
size_t selectedOption = 0;
bool menuOpen = true;
bool needDrawMenu = true;
bool optionSelected = false;
bool lockBtn = false;

std::array<size_t, 22> optionsValuesIdx {{ 
    2, 2, 2, 0, 1, 1, 0, 1, 0, 2, 2, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0
}};

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 500

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

    // pinMode(33, OUTPUT); // blink pin
    pinMode(buttonPin, INPUT_PULLDOWN);

    grayscale = Matrix<uint8_t>(320, 240);
    ditherMat = PrinterMatrix(320, 240);

    delay(200);
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
      20000000, // xclk_freq_hz // 20000000
      LEDC_TIMER_0, // ledc_timer
      LEDC_CHANNEL_0, // ledc_channel
      PIXFORMAT_JPEG, // pixel_format // PIXFORMAT_ + YUV422|GRAYSCALE|RGB565|RGB888|JPEG
      // Init with high specs to pre-allocate larger buffers 
      FRAMESIZE_VGA, // frame_size // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
      20, // jpeg_quality - 0-63 lower number means higher quality.
      2 // fb_count - number of frame buffers to be allocated. (if more than one, i2s runs in continuous mode. Use only with JPEG)
    };

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      log_d("[error] Camera init failed with error 0x%x", err);
      return;
    }

    s = esp_camera_sensor_get();
    // s->set_framesize(s, FRAMESIZE_VGA); // Drop down frame size to 640x480
    }

    tft.init();
    tft.setRotation(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(1);
    tft.setTextSize(1);
    
    utils::logMemory();

    for (size_t i = 0; i < 4; i++) {
        digitalWrite(33, HIGH); //Turn off
        delay (50); //Wait 1 sec
        digitalWrite(33, LOW); //Turn on
        delay (50); //Wait 1 sec
    }
    log_d("setup done.");
}

void buttonsActions() {
    // 208 1850 2920 3440 4095

    size_t btnId = buttonsValue/1000.f;

    if(!lockBtn) {
        switch (btnId) {
            case 0: // enter
                if(menuOpen) {
                    optionSelected = !optionSelected;
                }else {
                    menuOpen = true;
                }
                needDrawMenu = true;
                break;
            
            case 1: // back
                if(menuOpen) {
                    if(optionSelected) {
                        optionSelected = false;
                    }else {
                        menuOpen = false;
                    }
                    needDrawMenu = true;
                }
                break;

            case 2: // left
                if(menuOpen) {
                    if(optionSelected) {
                        optionsValuesIdx[selectedOption] = utils::saveModulo(optionsValuesIdx[selectedOption] + 1, menu::availableValuesByOptions[selectedOption].size());
                    } else {
                        selectedOption = utils::saveModulo(selectedOption + 1, menu::availableValuesByOptions.size());
                    }
                    needDrawMenu = true;
                }
                break;
            case 3: // right
                if(menuOpen) {
                    if(optionSelected) {
                        optionsValuesIdx[selectedOption] = utils::saveModulo(optionsValuesIdx[selectedOption] - 1, menu::availableValuesByOptions[selectedOption].size());
                    } else {
                        selectedOption = utils::saveModulo(selectedOption - 1, menu::availableValuesByOptions.size());
                    }
                    needDrawMenu = true;
                }
                
                break;
            default:
                break;
        }
    }
    lockBtn = btnId < 4;
}

void loop() {
    
    buttonsValue = analogRead(buttonPin);
    const size_t btnId = buttonsValue/1000.f;

    buttonsActions();

    if(menuOpen) {
        if(needDrawMenu) {
            menu::drawMenu(tft, selectedOption, optionSelected, optionsValuesIdx);
            needDrawMenu = false;
        }

        // display input
        // tft.setTextColor(TFT_BLUE, TFT_WHITE);
        // tft.drawNumber(buttonsValue, 100, tft.fontHeight(4), 4);
        // tft.drawNumber(btnId, 100, tft.fontHeight(4)*3, 4);

        delay(20);
    }else {
        if(needDrawMenu) {
            tft.fillScreen(TFT_WHITE);
            needDrawMenu = false;
        }
        
        for (size_t i = 0; i < grayscale.len(); ++i) {
            grayscale(i) = static_cast<uint8_t>(std::rand());
        }

        // convert frame buffer to matrix
        {
            camera_fb_t* fb = esp_camera_fb_get();
            log_d("[esp_camera_fb_get] pointer value: %p", fb);

            if(fb != nullptr) {
                Matrix<PIXELFORMAT_RGB> mat(fb->width, fb->height);
                log_d("buffer size: (%d, %d)\n",fb->width, fb->height);

                fbToMat(fb, mat);
                log_d("fbToMat");

                grayscale = utils::GrayRescaled(mat, 2.f);
                utils::logMemory();
            }
            esp_camera_fb_return(fb);
            log_d("esp_camera_fb_return");
        }
        
        // ditherMat = filtering::errorDiffusionPrinter(grayscale, 0.5f);
        
        utils::drawGrayScale(tft, 0, 0, grayscale);
        
       delay(WAIT);
    }
}
