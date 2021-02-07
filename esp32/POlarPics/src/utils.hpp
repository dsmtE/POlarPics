#pragma once 

#include <cstdint>

#include <TFT_eSPI.h>
#include <matrix.h>

namespace utils {
    
    template <typename T>
    inline int sgn(T val) { return (T(0) < val) - (val < T(0)); }

    template <typename T>
    inline T saveModulo(const T val, const T modulo) { return (modulo + val) % modulo; }

    void logPSRAM();

    void logHeap();

    void logMemory();

    uint16_t colorConverter(uint8_t r, uint8_t g, uint8_t b);
    inline uint16_t colorConverter(const uint8_t grey) { return colorConverter(grey, grey, grey); }

    void drawGrayScale(TFT_eSPI& tft, size_t x, size_t y, const Matrix<uint8_t>& map);

    Matrix<uint8_t> GrayRescaled(Matrix<PIXELFORMAT_RGB>& mat, const float ratio);
}