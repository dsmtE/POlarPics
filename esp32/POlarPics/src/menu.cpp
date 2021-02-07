#include "menu.hpp"

#include "utils.hpp"

namespace menu {
    const std::string optionToString(int option) {
        switch (option) {
            case CAM_OPTION_BRIGHTNESS :
                return "brightness";
            case CAM_OPTION_CONTRAST :
                    return "contrast";
            case CAM_OPTION_SATURATION :
                    return "saturation";
            case CAM_OPTION_SPECIAL_EFFECT :
                    return "special effect";
            case CAM_OPTION_WHITEBAL :
                    return "white balance";
            case CAM_OPTION_AWB_GAIN :
                    return "awb gain";
            case CAM_OPTION_WB_MODE :
                    return "wb_mode";
            case CAM_OPTION_EXPOSURE_CTRL :
                    return "exposure_ctrl";
            case CAM_OPTION_AEC2 :
                    return "aec2";
            case CAM_OPTION_AE_LEVEL :
                    return "ae_level";
            case CAM_OPTION_AEC_VALUE :
                    return "aec_value";
            case CAM_OPTION_GAIN_CTRL :
                    return "gain_ctrl";
            case CAM_OPTION_AGC_GAIN :
                    return "agc_gain";
            case CAM_OPTION_GAINCEILING :
                    return "gainceiling";
            case CAM_OPTION_BPC :
                    return "bpc";
            case CAM_OPTION_WPC :
                    return "wpc";
            case CAM_OPTION_RAW_GMA :
                    return "raw_gma";
            case CAM_OPTION_LENC :
                    return "lenc";
            case CAM_OPTION_HMIRROR :
                    return "hmirror";
            case CAM_OPTION_VFLIP :
                    return "vflip";
            case CAM_OPTION_DCW :
                    return "dcw";
            case CAM_OPTION_COLORBAR :
                    return "colorbar";
            default:
            throw std::runtime_error("Error sensor setting undefined");
                break;
        }
    }

    void setOption(sensor_t* s, int option, int value) {
        switch (option) {
            case CAM_OPTION_BRIGHTNESS:
                s->set_brightness(s, value);
                break;
            case CAM_OPTION_CONTRAST:
                s->set_contrast(s, value);
                break;
            case CAM_OPTION_SATURATION:
                s->set_saturation(s, value);
                break;
            case CAM_OPTION_SPECIAL_EFFECT:
                s->set_special_effect(s, value);
                break;
            case CAM_OPTION_WHITEBAL:
                s->set_whitebal(s, value);
                break;
            case CAM_OPTION_AWB_GAIN:
                s->set_awb_gain(s, value);
                break;
            case CAM_OPTION_WB_MODE:
                s->set_wb_mode(s, value);
                break;
            case CAM_OPTION_EXPOSURE_CTRL:
                s->set_exposure_ctrl(s, value);
                break;
            case CAM_OPTION_AEC2:
                s->set_aec2(s, value);
                break;
            case CAM_OPTION_AE_LEVEL:
                s->set_ae_level(s, value);
                break;
            case CAM_OPTION_AEC_VALUE:
                s->set_aec_value(s, value);
                break;
            case CAM_OPTION_GAIN_CTRL:
                s->set_gain_ctrl(s, value);
                break;
            case CAM_OPTION_AGC_GAIN:
                s->set_agc_gain(s, value);
                break;
            case CAM_OPTION_GAINCEILING:
                s->set_gainceiling(s, gainceiling_t(value));
                break;
            case CAM_OPTION_BPC:
                s->set_bpc(s, value);
                break;
            case CAM_OPTION_WPC:
                s->set_wpc(s, value);
                break;
            case CAM_OPTION_RAW_GMA:
                s->set_raw_gma(s, value);
                break;
            case CAM_OPTION_LENC:
                s->set_lenc(s, value);
                break;
            case CAM_OPTION_HMIRROR:
                s->set_hmirror(s, value);
                break;
            case CAM_OPTION_VFLIP:
                s->set_vflip(s, value);
                break;
            case CAM_OPTION_DCW:
                s->set_dcw(s, value);
                break;
            case CAM_OPTION_COLORBAR:
                s->set_colorbar(s, value);
                break;
            default:
            throw std::runtime_error("Error sensor setting undefined");
                break;
        }
    }

    std::string optionValueToString(int option, int value) {
        const std::vector<std::pair<std::string, int>> availableValues = availableValuesByOptions[option];
        const auto end = availableValues.end();
        const auto it = std::find_if(availableValues.begin(), end, [&value](const std::pair<std::string, int>& v) 
        { return v.second == value; } );
        
        if(it == end)
            throw std::runtime_error("[error] value for this option can't be found");

        return it->first;
    }

    void drawMenu(TFT_eSPI& tft, const size_t selectedOption, bool optionSelected, const std::array<size_t, 22>& optionsValuesIdx) {
        tft.fillScreen(TFT_WHITE);

        tft.setTextPadding(1);
        tft.setTextSize(1);

        const int lineHeight = tft.fontHeight(4);
        const int centerHeight = 120;
        const int padding = 4;
        
        // use array and https://en.cppreference.com/w/cpp/algorithm/rotate to accelerate the result
        for (int i = -3; i <= 3; ++i) {
            const bool isNotCenter = i != 0;
            const uint8_t greyLevel = 52*abs(i) + 32 * isNotCenter;
            const uint16_t color = utils::colorConverter(greyLevel, greyLevel, greyLevel);

            tft.setTextColor(i == 0 && optionSelected ? TFT_DARKCYAN : color, TFT_WHITE);

            const int id = (availableValuesByOptions.size() + selectedOption + i) % availableValuesByOptions.size();
            const int yPos = centerHeight + i * (lineHeight + padding) + isNotCenter * utils::sgn(i) * padding;
            tft.drawString(optionToString(id).c_str(), 90, yPos, 4);
            tft.drawString(optionValueToString(id, availableValuesByOptions[id][optionsValuesIdx[id]].second).c_str(), 240, yPos, 4);
        }
        tft.drawLine(10, centerHeight + (lineHeight + padding)/2 , 310, centerHeight + (lineHeight + padding)/2 , TFT_DARKGREY);
        tft.drawLine(10, centerHeight - lineHeight/2 - padding , 310, centerHeight - lineHeight/2 - padding , TFT_DARKGREY);
    }

}