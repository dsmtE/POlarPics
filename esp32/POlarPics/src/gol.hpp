#include <cstdint>

#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

namespace gol {

    #define MAX_GEN_COUNT 1000
    #define GRIDX 80 // 320/4
    #define GRIDY 60 // 240/4
    #define CELLXY 4
    #define GEN_DELAY 10 // Set a delay between each generation to slow things down

    void drawGrid(TFT_eSPI& tft, uint8_t** grid, uint8_t** newgrid);
    void initGrid(uint8_t** grid, uint8_t** newgrid);
    void computeCA( uint8_t** grid, uint8_t** newgrid);
    int getNumberOfNeighbors(int x, int y, uint8_t** grid);

    void golLoop(TFT_eSPI& tft);

 
}