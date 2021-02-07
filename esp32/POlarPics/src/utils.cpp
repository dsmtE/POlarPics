#include "utils.hpp"

#include <stdexcept>

#include "filtering.h"

namespace utils {

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

    uint16_t colorConverter(uint8_t r, uint8_t g, uint8_t b) {
        // R, G, and B are divided equally giving 3 * 5 = 15 bits and the additional bit is allocated to Green.
        return static_cast<uint16_t>(r >> 3) << 11 | static_cast<uint16_t>(g >> 2) << 5 | static_cast<uint16_t>(b >> 3);
    }

    void drawGrayScale(TFT_eSPI& tft, size_t x, size_t y, const Matrix<uint8_t>& map) {
        for (size_t r = 0; r < map.height(); ++r) {
            for (size_t c = 0; c < map.width(); ++c) {
                const uint8_t grey = map(r, c);
                tft.drawPixel(x + c, y + r, tft.color565(grey, grey, grey));
                // 320*240
            }
        }
    }

    Matrix<uint8_t> GrayRescaled(Matrix<PIXELFORMAT_RGB>& mat, const float ratio) {
        const size_t newWidth = mat.width() * ratio;
        const size_t newHeight = mat.height() * ratio;
        Matrix<uint8_t> out(newWidth, newHeight);
        for (size_t r = 0; r < newHeight; ++r) {
            for (size_t c = 0; c < newWidth; ++c) {
                size_t backwartR = (static_cast<float>(r) / static_cast<float>(newHeight) * static_cast<float>(mat.height()));
                size_t backwartC = (static_cast<float>(c) / static_cast<float>(newWidth) * static_cast<float>(mat.width()));
                out(r, c) = filtering::rgbToGrayscale(mat(backwartR, backwartC));
            }
        }
        return out;
    }

}
