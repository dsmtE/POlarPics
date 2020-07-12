#include "filtering.h"

#include <assert.h>
#include <algorithm>

uint8_t filtering::rgbToGrayscale(const uint8_t r, const uint8_t g, const uint8_t b) { 
    return static_cast<std::uint8_t>(0.299f * r + 0.587f * g + 0.144f * b);
}

uint8_t filtering::rgbToGrayscale(const PIXELFORMAT_RGB& pixel) {
    return rgbToGrayscale(pixel.r, pixel.g, pixel.b);
}

Matrix<uint8_t> filtering::convertToGrayscale(const Matrix<PIXELFORMAT_RGB> inMat) {

    Matrix<uint8_t> outMat(inMat.width(), inMat.height());
    // Apply grayscale conversion
    std::transform(inMat.data(), inMat.data() + inMat.len(), outMat.data(), [](const PIXELFORMAT_RGB& e) { return filtering::rgbToGrayscale(e); });
    /*
    for (size_t i = 0; i < inMat.len(); i++) {
        outMat[i] = filtering::rgbToGrayscale(inMat[i]);
    }
    */
    return outMat;
}

Matrix<uint8_t> filtering::errorDiffusion(const Matrix<uint8_t> inMat, const float threshold, EFilteringMethod method) {
    
    // copy
    Matrix<uint8_t> outMat(inMat);

    // get pattern
    const auto end = diffusionPatterns.end();
    const auto it = std::find_if(diffusionPatterns.begin(), end,[&method](const std::pair<EFilteringMethod, std::vector<std::array<float, 3>>>& e) 
    { return e.first ==method; } );
    
    // If method can't be found
    if( it == end)
        throw std::runtime_error("[error] errorDiffusion: pattenr can't be found.");

    const std::vector<std::array<float, 3>> pattern = it->second;

    const size_t rows = outMat.height();
    const size_t cols = outMat.width();
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            const float newPixel = 255.f * std::floor(outMat[i * cols + j] / (threshold * 255.0f)); // Quantization by threshold
            const float error = outMat[i * cols + j] - newPixel; // Error
            outMat[i * cols + j] = (uint8_t)newPixel; // Assignation

            // Error diffusion
            for (const std::array<float, 3> patternSplit : pattern) {
                const int deltaCols = (int)patternSplit[0];
                const int deltaRows = (int)patternSplit[1];
                const float proportion = patternSplit[2];
                if( i + deltaRows >= 0 and i + deltaRows < rows and j + deltaCols >= 0 and j + deltaCols < cols) {
                    outMat[(i + deltaRows) * cols + (j + deltaCols)] = (uint8_t)std::max(255.0f, outMat[(i + deltaRows) * cols + (j + deltaCols)] + error * proportion); // error diffusion
                }
            }
        }
    }
    return outMat;
}