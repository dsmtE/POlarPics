#include <Arduino.h> // Core

#include "esp_camera.h" // Cam

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

#include "img_converters.h"

// #include "driver/rtc_io.h" // allows lock GPIO state during sleep

#include <cstdlib>
#include <stdexcept>
#include <array>

#include "Matrix.h"
#include "PrinterMatrix.h"

#include "filtering.h"
#include "utils.hpp"
#include "menu.hpp"

#include "test.h" // img exemple
#include <HardwareSerial.h>
#include "Adafruit_Thermal.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define BUTTON_PIN 12

sensor_t* s = nullptr;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// matrix buffers
Matrix<uint8_t> grayscale;
// PrinterMatrix ditherMat;

// menuVariables
size_t buttonsValue;
size_t selectedOption = 0;
bool menuOpen = false;
bool needDrawMenu = false;
bool optionSelected = false;

size_t lockBtn = 0; // lock btn if != 0
const size_t lockIteration = 2;


std::array<size_t, 22> optionsValuesIdx {{
    2, 2, 2, 0, 1, 1, 0, 1, 0, 2, 2, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0
}};

//HardwareSerial printerSerial(1);
Adafruit_Thermal printer(&Serial); // Pass addr to printer constructor

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 100

void testImg(Adafruit_Thermal& printer) {
  printer.printBitmap(test_width, test_height, test_data);
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
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


    //Serial.begin(115200);

    Serial.begin(9600, SERIAL_8N1);

    //printerSerial.begin(9600, SERIAL_8N1, 2, 14);
    printer.begin();

    delay(WAIT);

    // pinMode(33, OUTPUT); // blink pin
    pinMode(BUTTON_PIN, INPUT);

    grayscale = Matrix<uint8_t>(320, 240);
    // ditherMat = PrinterMatrix(320, 240);

    bool psramFoundValue = psramFound();
    // if(psramFoundValue) Serial.printf("psramFound");

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
    config.frame_size = psramFoundValue ? FRAMESIZE_UXGA : FRAMESIZE_SVGA;
    config.jpeg_quality = psramFoundValue ? 10 : 12;
    config.fb_count = psramFoundValue ? size_t(2): size_t(1);
    //init with high specs to pre-allocate larger buffers
    // // camera pin configuration
    // const camera_config_t config = {
    //   PWDN_GPIO_NUM, // pin_pwdn
    //   RESET_GPIO_NUM, // pin_reset
    //   XCLK_GPIO_NUM, // pin_xclk
    //   SIOD_GPIO_NUM, // pin_sscb_sda
    //   SIOC_GPIO_NUM, // pin_sscb_scl
    //   Y9_GPIO_NUM, // pin_d7
    //   Y8_GPIO_NUM, // pin_d6
    //   Y7_GPIO_NUM, // pin_d5
    //   Y6_GPIO_NUM, // pin_d4
    //   Y5_GPIO_NUM, // pin_d3
    //   Y4_GPIO_NUM, // pin_d2
    //   Y3_GPIO_NUM, // pin_d1
    //   Y2_GPIO_NUM, // pin_d0
    //   VSYNC_GPIO_NUM, // pin_vsync
    //   HREF_GPIO_NUM, // pin_href
    //   PCLK_GPIO_NUM, // pin_pclk
    //   20000000, // xclk_freq_hz // 20000000
    //   LEDC_TIMER_0, // ledc_timer
    //   LEDC_CHANNEL_0, // ledc_channel
    //   PIXFORMAT_RGB565, // pixel_format // PIXFORMAT_ + YUV422|GRAYSCALE|RGB565|RGB888|JPEG
    //   // Init with high specs to pre-allocate larger buffers 
    //   FRAMESIZE_QVGA, // frame_size // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    //   12, // jpeg_quality - 0-63 lower number means higher quality.
    //   2 // fb_count - number of frame buffers to be allocated. (if more than one, i2s runs in continuous mode. Use only with JPEG)
    // };

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        //Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA); //drop down frame size for higher initial frame rate
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }

    // Screen initialisation
    tft.init();
    tft.setRotation(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(1);
    tft.setTextSize(1);
    
    utils::logMemory();

    // for (size_t i = 0; i < 4; i++) {
    //     digitalWrite(33, HIGH); //Turn off
    //     delay (50); //Wait 1 sec
    //     digitalWrite(33, LOW); //Turn on
    //     delay (50); //Wait 1 sec
    // }
    //Serial.printf("setup done.");
}

size_t buttonMap(size_t value) {
    const size_t TheoricalBtnValues[4] = {730, 1360, 2500, 4050};
    size_t margin = 200;

    for (size_t i = 0; i < 4; ++i) {
        if(value > (TheoricalBtnValues[i] - margin) && value < (TheoricalBtnValues[i] + margin)) return i+1;
    }
    
    return 0;
}

void buttonsActions(const size_t btnValue) {
    size_t btnId = buttonMap(btnValue);

    if(lockBtn == 0) {
        switch (btnId) {
            case 1: // enter
                if(menuOpen) {
                    optionSelected = !optionSelected;
                }else {
                    menuOpen = true;
                }
                needDrawMenu = true;
                break;
            
            case 2: // back
                if(menuOpen) {
                    if(optionSelected) {
                        optionSelected = false;
                    }else {
                        menuOpen = false;
                    }
                    needDrawMenu = true;
                }
                break;

            case 3: // left
                if(menuOpen) {
                    if(optionSelected) {
                        optionsValuesIdx[selectedOption] = utils::saveModulo(optionsValuesIdx[selectedOption] + 1, menu::availableValuesByOptions[selectedOption].size());
                        menu::setOption(s, selectedOption, menu::availableValuesByOptions[selectedOption][optionsValuesIdx[selectedOption]].second);
                    } else {
                        selectedOption = utils::saveModulo(selectedOption + 1, menu::availableValuesByOptions.size());
                    }
                    needDrawMenu = true;
                }else {
                    testImg(printer);
                }
                break;
            case 4: // right
                if(menuOpen) {
                    if(optionSelected) {
                        optionsValuesIdx[selectedOption] = utils::saveModulo(optionsValuesIdx[selectedOption] - 1, menu::availableValuesByOptions[selectedOption].size());

                        menu::setOption(s, selectedOption, menu::availableValuesByOptions[selectedOption][optionsValuesIdx[selectedOption]].second);
                    } else {
                        selectedOption = utils::saveModulo(selectedOption - 1, menu::availableValuesByOptions.size());
                    }
                    needDrawMenu = true;
                }
                
                break;
            default:
                break;
        }
        lockBtn = lockIteration;
    }else {
        --lockBtn;
    }
}

void loop() {
    
    buttonsValue = analogRead(BUTTON_PIN);
    buttonsActions(buttonsValue);

    if(menuOpen) {
        if(needDrawMenu) {
            menu::drawMenu(tft, selectedOption, optionSelected, optionsValuesIdx);
            needDrawMenu = false;
        }
    } else {
        
        // random noise
        for (size_t i = 0; i < grayscale.len(); ++i) {
            grayscale(i) = static_cast<uint8_t>(std::rand());
        }

        delay(WAIT);

        camera_fb_t* fb = esp_camera_fb_get();
        // esp_err_t res = ESP_OK;
        // int64_t fr_start = esp_timer_get_time();

        //Serial.printf("[esp_camera_fb_get] pointer value: %p\n", fb);
        if (fb == nullptr) {
            //Serial.printf("Camera capture failed\n");
        } else {
            //Serial.printf("success: buffer size: (%d, %d)\n",fb->width, fb->height);

            Matrix<PIXELFORMAT_RGB> mat(fb->width, fb->height);
            //Serial.printf("buffer size: (%d, %d)\n",fb->width, fb->height);

            fbToMat(fb, mat);
            //Serial.printf("fbToMat");

            // grayscale = utils::GrayRescaled(mat, 2.f);
            grayscale = filtering::convertToGrayscale(mat);
            // utils::logMemory();
        }
        //Serial.printf("[esp_camera_fb_return]");
        esp_camera_fb_return(fb);

        // ditherMat = filtering::errorDiffusionPrinter(grayscale, 0.5f);
        utils::drawGrayScale(tft, 0, 0, grayscale);
        
       delay(WAIT);
    }

    // // display input
    // tft.setTextColor(TFT_BLUE, TFT_WHITE);
    // tft.drawNumber(buttonsValue, 100, tft.fontHeight(4), 4);
    // tft.drawNumber(buttonMap(buttonsValue), 100, tft.fontHeight(4)*3, 4);
}
