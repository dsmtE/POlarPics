#pragma once

#include <string>
#include <array>
#include <string>
#include <vector>

#include "esp_camera.h" // Cam
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

// Define index for cam options
#define CAM_OPTION_BRIGHTNESS  0
#define CAM_OPTION_CONTRAST  1
#define CAM_OPTION_SATURATION  2
#define CAM_OPTION_SPECIAL_EFFECT  3
#define CAM_OPTION_WHITEBAL  4
#define CAM_OPTION_AWB_GAIN  5
#define CAM_OPTION_WB_MODE  6
#define CAM_OPTION_EXPOSURE_CTRL  7
#define CAM_OPTION_AEC2  8
#define CAM_OPTION_AE_LEVEL  9
#define CAM_OPTION_AEC_VALUE  10
#define CAM_OPTION_GAIN_CTRL  11
#define CAM_OPTION_AGC_GAIN  12
#define CAM_OPTION_GAINCEILING  13
#define CAM_OPTION_BPC  14
#define CAM_OPTION_WPC  15
#define CAM_OPTION_RAW_GMA  16
#define CAM_OPTION_LENC  17
#define CAM_OPTION_HMIRROR  18
#define CAM_OPTION_VFLIP  19
#define CAM_OPTION_DCW  20
#define CAM_OPTION_COLORBAR  21

namespace menu {
    
    static const std::array<std::vector<std::pair<std::string, int>>, 22> availableValuesByOptions {{ 
        {{"-2", -2}, {"-1", -1}, {"0", 0}, {"1", 1}, {"2", 2}}, // brightness (-2 to 2)
        {{"-2", -2}, {"-1", -1}, {"0", 0}, {"1", 1}, {"2", 2}}, // contrast (-2 to 2)
        {{"-2", -2}, {"-1", -1}, {"0", 0}, {"1", 1}, {"2", 2}}, // saturation (-2 to 2)
        {{"No Effect", 0}, {"Negative", 1}, {"Grayscale", 2}, {"Red Tint", 3}, {"Green Tint", 4}, {"Blue Tint", 5}, {"Sepia", 6}}, // special_effect (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        {{"disable", 0}, {"enable", 1}}, // whitebal (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // awb_gain // 0 = disable , 1 = enable)
        {{"Auto", 0}, {"Sunny", 1}, {"Cloudy", 2}, {"Office", 3}, {"Home", 4}}, // wb_mod if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        {{"disable", 0}, {"enable", 1}}, // exposure_ctrl (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // aec2 (0 = disable , 1 = enable)
        {{"-2", -2}, {"-1", -1}, {"0", 0}, {"1", 1}, {"2", 2}}, // ae_level (-2 to 2)
        {{"0", 0}, {"100", 100}, {"200", 200}, {"300", 300}, {"400", 400}, {"500", 500}, {"600", 600}, {"700", 700}, {"800", 800}, {"900", 900}, {"1000", 1000}, {"1100", 1100}, {"1200", 1200}}, // aec_value (0 to 1200)
        {{"disable", 0}, {"enable", 1}}, // gain_ctrl (0 = disable , 1 = enable)
        {{"0", 0}, {"3", 3}, {"6", 6}, {"9", 9}, {"12", 12}, {"15", 15}, {"18", 18}, {"21", 21}, {"24", 24}, {"27", 27}, {"30", 30}}, // agc_gain (0 to 30)
        {{"x2", 0}, {"x4", 1}, {"x8", 2}, {"x16", 3}, {"x32", 4}, {"x64", 5}, {"x128", 6}}, // gainceiling (0 - x2, 1 - x4, 2 - x8, 3 - x16, 4 - x32, 5 - x64, 6 - x128)
        {{"disable", 0}, {"enable", 1}}, // bpc (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // wpc (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // raw_gma (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // lenc (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // hmirror (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // vflip (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}}, // dcw (0 = disable , 1 = enable)
        {{"disable", 0}, {"enable", 1}} // colorbar (0 = disable , 1 = enable)
    }};

    const std::string optionToString(int option);

    void setOption(sensor_t* s, int option, int value);

    std::string optionValueToString(int option, int value);

    void drawMenu(TFT_eSPI& tft, const size_t selectedOption, bool optionSelected, const std::array<size_t, 22>& optionsValuesIdx);
}