#pragma once 

#include <Matrix.h>

#include <vector>
#include <array>
#include <unordered_map>

namespace filtering
{

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

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

const std::array<std::pair<EFilteringMethod, std::vector<std::array<float, 3>>>, 9> diffusionPatterns {{
    {EFilteringMethod::floydSteinberg,
        { {1, 0,  7 / 16}, {-1, 1, 3 / 16}, {0, 1,  5 / 16}, {1, 1,  1 / 16} }
    },
    {EFilteringMethod::atkinson,
        { {1, 0,  1 / 8}, {2, 0,  1 / 8}, {-1, 1, 1 / 8}, {0, 1,  1 / 8}, {1, 1,  1 / 8}, {0, 2,  1 / 8} }
    },
    {EFilteringMethod::jarvisJudiceNinke,
        { {1, 0,  7 / 48}, {2, 0,  5 / 48}, {-2, 1, 3 / 48}, {-1, 1, 5 / 48}, {0, 1,  7 / 48}, {1, 1,  5 / 48}, {2, 1,  3 / 48}, {-2, 2, 1 / 48}, {-1, 2, 3 / 48}, {0, 2,  5 / 48}, {1, 2,  3 / 48}, {2, 2,  1 / 48} }
    },
    {EFilteringMethod::stucki,
        { {1, 0,  8 / 42}, {2, 0,  4 / 42}, {-2, 1, 2 / 42}, {-1, 1, 4 / 42}, {0, 1,  8 / 42}, {1, 1,  4 / 42}, {2, 1,  2 / 42}, {-2, 2, 1 / 42}, {-1, 2, 2 / 42}, {0, 2,  4 / 42}, {1, 2,  2 / 42}, {2, 2,  1 / 42} }
    },
    {EFilteringMethod::burkes,
        { {1, 0,  8 / 32}, {2, 0,  4 / 32}, {-2, 1, 2 / 32}, {-1, 1, 4 / 32}, {0, 1,  8 / 32}, {1, 1,  4 / 32}, {2, 1,  2 / 32} }
    },
    {EFilteringMethod::sierra3,
        { {1, 0,  5 / 32}, {2, 0,  3 / 32}, {-2, 1, 2 / 32}, {-1, 1, 4 / 32}, {0, 1,  5 / 32}, {1, 1,  4 / 32}, {2, 1,  2 / 32}, {-1, 2, 2 / 32}, {0, 2,  3 / 32}, {1, 2,  2 / 32} }
    },
    {EFilteringMethod::sierra2,
        { {1, 0,  4 / 16}, {2, 0,  3 / 16}, {-2, 1, 1 / 16}, {-1, 1, 2 / 16}, {0, 1,  3 / 16}, {1, 1,  2 / 16}, {2, 1,  1 / 16} }
    },
    {EFilteringMethod::sierra24a,
        { {1, 0,  2 / 4}, {-1, 1, 1 / 4}, {0, 1,  1 / 4} }
    },
    {EFilteringMethod::stevensonArce, 
        { {2, 0,   32 / 200}, {-3, 1,  12 / 200}, {-1, 1,  26 / 200}, {1, 1,   30 / 200}, {3, 1,   30 / 200}, {-2, 2,  12 / 200}, {0, 2,   26 / 200}, {2, 2,   12 / 200}, {-3, 3,   5 / 200}, {-1, 3,  12 / 200}, {1, 3,   12 / 200}, {3, 3,    5 / 200} }
    }
}};

uint8_t rgbToGrayscale(const uint8_t r, const uint8_t g, const uint8_t b);
uint8_t rgbToGrayscale(const PIXELFORMAT_RGB& pixel);

Matrix<uint8_t> convertToGrayscale(const Matrix<PIXELFORMAT_RGB> inMat);

Matrix<uint8_t> errorDiffusion(const Matrix<uint8_t> inMat, const float threshold = 0.5f, EFilteringMethod method = EFilteringMethod::floydSteinberg);

} // namespace filtering