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
    //log_d("input matrix width/height : %d/%d ", inMat.width(), inMat.height());
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
                if(newi >= 0 && newi < rows && newj >= 0 && newj < cols)
                    mat(newi, newj) = static_cast<uint8_t>(clamp(mat(newi, newj) + error * ps.proportion, 0.0f, 255.0f)); // error diffusion
            }
        }
    }
}

PrinterMatrix filtering::errorDiffusionPrinter(Matrix<uint8_t>& mat, const float threshold, const EFilteringMethod method) {

    // get pattern
    const auto end = diffusionPatterns.end();
    const auto it = std::find_if(diffusionPatterns.begin(), end, [&method](const std::pair<EFilteringMethod, std::vector<PatternPart>>& e) 
    { return e.first == method; } );
    
    // If method can't be found
    if(it == end) throw std::runtime_error("[error] errorDiffusion: pattern can't be found.");

    const std::vector<PatternPart> pattern = it->second;

    const size_t cols = mat.width();
    const size_t rows = mat.height();
    PrinterMatrix out(cols, rows);
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            const float pixelValue = static_cast<float>(mat(r, c));
            const bool newPixel = pixelValue > (threshold * 255.f); // Quantization by threshold
            const float error = pixelValue - (newPixel ? 255.f : 0.f); // Error
            out.set(r, c, newPixel); // Assignation

            // Error diffusion
            for (const PatternPart ps : pattern) {
                const int newRow = r + ps.deltaRows;
                const int newCol = c + ps.deltaCols;
                if(newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                    mat(newRow, newCol) = static_cast<uint8_t>(clamp<float>(static_cast<float>(mat(newRow, newCol)) + error * ps.proportion, 0.0f, 255.0f)); // error diffusion
                }
            }
        }
    }

    // for (size_t i = 0; i < mat.height(); ++i) {
    //     for (size_t j = 0; j < mat.width(); ++j) {
    //         out.set(i, j, i%2 == 0);
    //     }
    // }
    
    return out;
}