#include "esp_camera.h"
#include <Arduino.h>

// #include "SPIFFS.h"

#include "FS.h"               // SD Card ESP32
#include "SD_MMC.h"           // SD Card ESP32

#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

// #include <WiFi.h>
#include "img_converters.h"   
#include "dl_lib_matrix3d.h"  // image processing

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

const char* ssid = "";
const char* password = "";
// void startCameraServer();

void startCameraServer();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // Serial.println();

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
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Init SD Card
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  
  sensor_t * s = esp_camera_sensor_get();
  //drop down frame size for higher initial frame rate
  // s->set_framesize(s, FRAMESIZE_QVGA);

  // Take Picture with Camera
  camera_fb_t* fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Path where new picture will be saved in SD Card
  const String path = "/picture.jpg";

  // test write img to SD Card
  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    // EEPROM.write(0, pictureNumber);
    // EEPROM.commit();
  }
  file.close();

  esp_camera_fb_return(fb); 
  
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
  /*
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  */
}

// return NULL pointer if error
static dl_matrix3du_t* getImageMatrix() {
    camera_fb_t* fb = NULL;

    // get esp32 framebuffer
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("[Error] Getting camera frame buffer failed.");
        return NULL;
    }

    const size_t outWidth = fb->width;
    const size_t outHeight = fb->height;
    const size_t outLenght = fb->width * fb->height * 3;
    Serial.printf("len: %d, width: %d, height: %d\n", outLenght, outWidth, outHeight);

    // allocate matrix buffer
    dl_matrix3du_t* imageMatrix = dl_matrix3du_alloc(1, outWidth, outHeight, 3);
    if (!imageMatrix) {
        // release frame buffer
        esp_camera_fb_return(fb);
        Serial.println("dl_matrix3du_alloc failed");
        return NULL;
    }
    uint8_t* outBuffer = imageMatrix->item;
    // Load img and store it into our buffer
    if(!fmt2rgb888(fb->buf, fb->len, fb->format, outBuffer)){
        dl_matrix3du_free(imageMatrix);
        Serial.println("[Error] conversion to rgb888 from framebuffer failed.");
        return NULL;
    }
    return imageMatrix;
}


void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
