#include "filtering.h"

#include <assert.h>
#include <algorithm>

uint8_t filtering::rgbToGrayscale(const uint8_t r, const uint8_t g, const uint8_t b) { 
    return std::max(0.0f, std::min(255.0f, 0.299f * float(r) + 0.587f * float(g) + 0.144f * float(b)));
}

uint8_t filtering::rgbToGrayscale(const PIXELFORMAT_RGB& pixel) {
    return rgbToGrayscale(pixel.r, pixel.g, pixel.b);
}

Matrix<uint8_t> filtering::convertToGrayscale(const Matrix<PIXELFORMAT_RGB>& inMat) {

    Matrix<uint8_t> outMat(inMat.width(), inMat.height());
    // Apply grayscale conversion
    std::transform(inMat.begin(), inMat.end(), outMat.data(), [](const PIXELFORMAT_RGB& e) { return filtering::rgbToGrayscale(e); });
    
    return outMat;
}

void filtering::errorDiffusion(Matrix<uint8_t>& mat, const float threshold, EFilteringMethod method) {

    // get pattern
    const auto end = diffusionPatterns.end();
    const auto it = std::find_if(diffusionPatterns.begin(), end, [&method](const std::pair<EFilteringMethod, std::vector<PatternPart>>& e) 
    { return e.first == method; } );
    
    // If method can't be found
    if(it == end)
        throw std::runtime_error("[error] errorDiffusion: pattern can't be found.");

    const std::vector<PatternPart> pattern = it->second;

    const size_t cols = mat.width();
    const size_t rows = mat.height();
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            const float newPixel = 255.f * std::floor(mat(i,j) / (threshold * 255.0f)); // Quantization by threshold
            const float error = mat(i,j) - newPixel; // Error
            mat(i,j) = static_cast<uint8_t>(newPixel); // Assignation

            // Error diffusion
            for (const PatternPart ps : pattern) {
                const size_t newi = i + ps.deltaRows;
                const size_t newj = j + ps.deltaCols;
                if(newi >= 0 and newi < rows and newj >= 0 and newj < cols)
                    mat(newi, newj) = static_cast<uint8_t>(clamp(mat(newi, newj) + error * ps.proportion, 0.0f, 255.0f)); // error diffusion
            }
        }
    }
}