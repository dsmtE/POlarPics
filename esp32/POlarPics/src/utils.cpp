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

    void drawGrayScale(TFT_eSPI& tft, size_t x, size_t y, const Matrix<uint8_t>& mat) {
        for (size_t r = 0; r < mat.height(); ++r) {
            for (size_t c = 0; c < mat.width(); ++c) {
                const uint8_t grey = mat(r, c);
                tft.drawPixel(x + c, y + r, tft.color565(grey, grey, grey));
                // 320*240
            }
        }
    }

    void drawGrayScale(TFT_eSPI& tft, size_t x, size_t y, const PrinterMatrix& mat) {
        for (size_t r = 0; r < mat.height(); ++r) {
            for (size_t c = 0; c < mat.width(); ++c) {
                const uint8_t grey = mat(r, c) ? 255 : 0;
                tft.drawPixel(x + c, y + r, tft.color565(grey, grey, grey));
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

// template <typename T>
// Matrix<T> rescale(const Matrix<T>& in, const int8_t ratioNum, const int8_t ratioDenom) {
//     const size_t newWidth = in.width() * ratioNum / ratioDenom;
//     const size_t newHeight = in.height() * ratioNum / ratioDenom;
//     Matrix<T> out(newWidth, newHeight);
//     for (size_t r = 0; r < newHeight; ++r) {
//         for (size_t c = 0; c < newWidth; ++c) {
//             std::vector<T> values(ratioDenom*ratioDenom);
//             // compute subKernel input values
//             for (size_t j = 0; j < ratioDenom; ++j) {
//                 for (size_t i = 0; i < ratioDenom; ++i) {
//                     values[i + j * ratioDenom] = in((r * ratioDenom + i) / ratioNum, (c* ratioDenom + j) / ratioNum);
//                 }
//             }
//             // here simply take the average of input values instead of apply a specific kernel
//             T sum = std::accumulate(values.begin(), values.end(), T(0));
//             out(r, c) = sum / ratioDenom*ratioDenom;
//         }
//     }
//     return out;
// }
