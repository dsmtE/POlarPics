#pragma once 

#include <Matrix.h>

#include <vector>
#include <array>
#include <unordered_map>

namespace filtering {

enum class EFilteringMethod {
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

template <typename T>
inline T clamp(const T& value, const T& min, const T& max) { return std::max(min, std::min(max, value)); }

struct PatternPart {
    int deltaCols;
    int deltaRows;
    float proportion;
};

const std::array<std::pair<EFilteringMethod, std::vector<PatternPart>>, 9> diffusionPatterns {{
    {EFilteringMethod::floydSteinberg,
        { {1, 0,  7.0f / 16.0f}, {-1, 1, 3.0f / 16.0f}, {0, 1,  5.0f / 16.0f}, {1, 1,  1.0f / 16.0f} }
    },
    {EFilteringMethod::atkinson,
        { {1, 0,  1.0f / 8.0f}, {2, 0,  1.0f / 8.0f}, {-1, 1, 1.0f / 8.0f}, {0, 1,  1.0f / 8.0f}, {1, 1,  1.0f / 8.0f}, {0, 2,  1.0f / 8.0f} }
    },
    {EFilteringMethod::jarvisJudiceNinke,
        { {1, 0,  7.0f / 48.0f}, {2, 0,  5.0f / 48.0f}, {-2, 1, 3.0f / 48.0f}, {-1, 1, 5.0f / 48.0f}, {0, 1,  7.0f / 48.0f}, {1, 1,  5.0f / 48.0f}, {2, 1,  3.0f / 48.0f}, {-2, 2, 1.0f / 48.0f}, {-1, 2, 3.0f / 48.0f}, {0, 2,  5.0f / 48.0f}, {1, 2,  3.0f / 48.0f}, {2, 2,  1.0f / 48.0f} }
    },
    {EFilteringMethod::stucki,
        { {1, 0,  8.0f / 42.0f}, {2, 0,  4.0f / 42.0f}, {-2, 1, 2.0f / 42.0f}, {-1, 1, 4.0f / 42.0f}, {0, 1,  8.0f / 42.0f}, {1, 1,  4.0f / 42.0f}, {2, 1,  2.0f / 42.0f}, {-2, 2, 1.0f / 42.0f}, {-1, 2, 2.0f / 42.0f}, {0, 2,  4.0f / 42.0f}, {1, 2,  2.0f / 42.0f}, {2, 2,  1.0f / 42.0f} }
    },
    {EFilteringMethod::burkes,
        { {1, 0,  8.0f / 32.0f}, {2, 0,  4.0f / 32.0f}, {-2, 1, 2.0f / 32.0f}, {-1, 1, 4.0f / 32.0f}, {0, 1,  8.0f / 32.0f}, {1, 1,  4.0f / 32.0f}, {2, 1,  2.0f / 32.0f} }
    },
    {EFilteringMethod::sierra3,
        { {1, 0,  5.0f / 32.0f}, {2, 0,  3.0f / 32.0f}, {-2, 1, 2.0f / 32.0f}, {-1, 1, 4.0f / 32.0f}, {0, 1,  5.0f / 32.0f}, {1, 1,  4.0f / 32.0f}, {2, 1,  2.0f / 32.0f}, {-1, 2, 2.0f / 32.0f}, {0, 2,  3.0f / 32.0f}, {1, 2,  2.0f / 32.0f} }
    },
    {EFilteringMethod::sierra2,
        { {1, 0,  4.0f / 16.0f}, {2, 0,  3.0f / 16.0f}, {-2, 1, 1.0f / 16.0f}, {-1, 1, 2.0f / 16.0f}, {0, 1,  3.0f / 16.0f}, {1, 1,  2.0f / 16.0f}, {2, 1,  1.0f / 16.0f} }
    },
    {EFilteringMethod::sierra24a,
        { {1, 0,  2.0f / 4.0f}, {-1, 1, 1.0f / 4.0f}, {0, 1,  1.0f / 4.0f} }
    },
    {EFilteringMethod::stevensonArce, 
        { {2, 0,   32.0f / 200.0f}, {-3, 1,  12.0f / 200.0f}, {-1, 1,  26.0f / 200.0f}, {1, 1,   30.0f / 200.0f}, {3, 1,   30.0f / 200.0f}, {-2, 2,  12.0f / 200.0f}, {0, 2,   26.0f / 200.0f}, {2, 2,   12.0f / 200.0f}, {-3, 3,   5.0f / 200.0f}, {-1, 3,  12.0f / 200.0f}, {1, 3,   12.0f / 200.0f}, {3, 3,    5.0f / 200.0f} }
    }
}};

uint8_t rgbToGrayscale(const uint8_t r, const uint8_t g, const uint8_t b);
uint8_t rgbToGrayscale(const PIXELFORMAT_RGB& pixel);

Matrix<uint8_t> convertToGrayscale(const Matrix<PIXELFORMAT_RGB>& inMat);

void errorDiffusion(Matrix<uint8_t>& mat, const float threshold = 0.5f, EFilteringMethod method = EFilteringMethod::floydSteinberg);

}// namespace filtering