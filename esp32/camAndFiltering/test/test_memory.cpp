#include <Arduino.h>
#include <unity.h>
#include "esp_camera.h"

#include <filtering.h>

// Pin definition for AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

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

void setup() {
    // Wait for >2 secs if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); // IMPORTANT LINE!

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
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_CIF); // 400x296

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[error] Camera init failed with error 0x%x", err);
        return;
    }

    // Get frame buffer from esp32 camera
    camera_fb_t *fb = esp_camera_fb_get();
    TEST_ASSERT_NOT_NULL(fb);

    TEST_ASSERT_EQUAL_size_t(fb->width, 400);
    TEST_ASSERT_EQUAL_size_t(fb->height, 296);

    const size_t fbRgbSize = fb->width * fb->height * 3;
    TEST_ASSERT_EQUAL_size_t(fbRgbSize, 400 * 296 * 3);

    byte* heapBuffer = (byte*) malloc(fbRgbSize);
    TEST_ASSERT_NULL(heapBuffer );
    free(heapBuffer);

    uint8_t* tmpBuffer = (uint8_t*) ps_malloc(fbRgbSize);
    TEST_ASSERT_NOT_NULL(tmpBuffer);
    free(tmpBuffer);

    UNITY_END(); // stop unit testing
}

void loop() {}