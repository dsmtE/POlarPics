#include "esp_camera.h"
#include <Arduino.h>
#include <vector>
#include <unordered_map>

enum class EMethod {
    floydSteinberg,
    atkinson,
    jarvisJudiceNinke,
    stucki,
    burkes,
    sierra3,
    sierra2,
    sierra24a,
    stevensonArce
};

static std::unordered_map<EMethod, std::vector<std::array<float, 3>>> diffusionPatterns {{
    {EMethod::floydSteinberg,
        { {1, 0,  7 / 16}, {-1, 1, 3 / 16}, {0, 1,  5 / 16}, {1, 1,  1 / 16} }
    },
    {EMethod::atkinson,
        { {1, 0,  1 / 8}, {2, 0,  1 / 8}, {-1, 1, 1 / 8}, {0, 1,  1 / 8}, {1, 1,  1 / 8}, {0, 2,  1 / 8} }
    },
    {EMethod::jarvisJudiceNinke,
        { {1, 0,  7 / 48}, {2, 0,  5 / 48}, {-2, 1, 3 / 48}, {-1, 1, 5 / 48}, {0, 1,  7 / 48}, {1, 1,  5 / 48}, {2, 1,  3 / 48}, {-2, 2, 1 / 48}, {-1, 2, 3 / 48}, {0, 2,  5 / 48}, {1, 2,  3 / 48}, {2, 2,  1 / 48} }
    },
    {EMethod::stucki,
        { {1, 0,  8 / 42}, {2, 0,  4 / 42}, {-2, 1, 2 / 42}, {-1, 1, 4 / 42}, {0, 1,  8 / 42}, {1, 1,  4 / 42}, {2, 1,  2 / 42}, {-2, 2, 1 / 42}, {-1, 2, 2 / 42}, {0, 2,  4 / 42}, {1, 2,  2 / 42}, {2, 2,  1 / 42} }
    },
    {EMethod::burkes,
        { {1, 0,  8 / 32}, {2, 0,  4 / 32}, {-2, 1, 2 / 32}, {-1, 1, 4 / 32}, {0, 1,  8 / 32}, {1, 1,  4 / 32}, {2, 1,  2 / 32} }
    },
    {EMethod::sierra3,
        { {1, 0,  5 / 32}, {2, 0,  3 / 32}, {-2, 1, 2 / 32}, {-1, 1, 4 / 32}, {0, 1,  5 / 32}, {1, 1,  4 / 32}, {2, 1,  2 / 32}, {-1, 2, 2 / 32}, {0, 2,  3 / 32}, {1, 2,  2 / 32} }
    },
    {EMethod::sierra2,
        { {1, 0,  4 / 16}, {2, 0,  3 / 16}, {-2, 1, 1 / 16}, {-1, 1, 2 / 16}, {0, 1,  3 / 16}, {1, 1,  2 / 16}, {2, 1,  1 / 16} }
    },
    {EMethod::sierra24a,
        { {1, 0,  2 / 4}, {-1, 1, 1 / 4}, {0, 1,  1 / 4} }
    },
    {EMethod::stevensonArce, 
        { {2, 0,   32 / 200}, {-3, 1,  12 / 200}, {-1, 1,  26 / 200}, {1, 1,   30 / 200}, {3, 1,   30 / 200}, {-2, 2,  12 / 200}, {0, 2,   26 / 200}, {2, 2,   12 / 200}, {-3, 3,   5 / 200}, {-1, 3,  12 / 200}, {1, 3,   12 / 200}, {3, 3,    5 / 200} }
    }
}};

// work only with grayScale img
static bool errorDifusion(uint8_t* inBuffer, const size_t cols, const size_t rows, uint8_t* outBuffer, const float threshold = 0.5, EMethod method = EMethod::floydSteinberg) {
    
    // allocate and copy data in outputBuffer if needed
    if(outBuffer != inBuffer) {
    
        free(outBuffer);

        // allocate output
        if( outBuffer == NULL) {
            outBuffer = (uint8_t*)malloc(cols * rows * sizeof(uint8_t));
            if( outBuffer == NULL) {
                // bad Allocation
                return false;
            }
        }

        memcpy (inBuffer, outBuffer, cols * rows);
    }

    // get pattern
    const std::vector<std::array<float, 3>> pattern = diffusionPatterns[method];

    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            const float newPixel = 255.f * std::floor(inBuffer[i * cols + j] / (threshold * 255.0f)); // Quantization by threshold
            const float error = inBuffer[i * cols + j] - newPixel; // error
            outBuffer[i * cols + j] = (uint8_t)newPixel; // assignation

            for (const std::array<float, 3> patternSplit : pattern) {
                const int deltaCols = (int)patternSplit[0];
                const int deltaRows = (int)patternSplit[1];
                const float proportion = patternSplit[2];
                if( i + deltaRows >= 0 and i + deltaRows < rows and j + deltaCols >= 0 and j + deltaCols < cols) {
                    outBuffer[(i + deltaRows) * cols + (j + deltaCols)] += error * proportion; // error diffusion
                }
            }
        }
    }
    return true;
}

/*
    pattern = diffusionPatterns.get(method.lower())
    # print(pattern)
    rows, cols = image.shape
    copy = np.copy(image)
    for i in range(0, rows):
        for j in range(0, cols):
            newPixel = 255 * np.floor(copy[i][j] / 128) # Quantization by threshold
            error = copy[i][j] - newPixel # error
            copy[i][j] = newPixel
            for deltaCols, deltaRows, proportion in pattern: # pattern destructuring
                if( i + deltaRows >= 0 and i + deltaRows < rows and j + deltaCols >= 0 and j + deltaCols < cols) :
                    copy[i + deltaRows][j + deltaCols] += error * proportion # error difusion
                    #  copy[i + deltaRows][j + deltaCols] += min(max(error * proportion, 0), 255) # error difusion

    return copy
    */