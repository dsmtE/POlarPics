#include "gol.hpp"

namespace gol {

    void golLoop(TFT_eSPI& tft) {
        //Display a simple splash screen
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(40, 5);
        tft.println(F("Arduino"));
        tft.setCursor(35, 25);
        tft.println(F("Cellular"));
        tft.setCursor(35, 45);
        tft.println(F("Automata"));

        delay(1000);

        tft.fillScreen(TFT_BLACK);

        uint8_t** grid = new uint8_t*[GRIDY];
        uint8_t** newgrid = new uint8_t*[GRIDY];
        for (int row = 0; row < GRIDY; ++row) {
            grid[row] = new uint8_t[GRIDX];
            newgrid[row] = new uint8_t[GRIDX];
        }

        initGrid(grid, newgrid);

        drawGrid(tft, grid, newgrid);

        //Compute generations
        for (size_t gen = 0; gen < MAX_GEN_COUNT; gen++) {
            computeCA(grid, newgrid);
            drawGrid(tft, grid, newgrid);
            delay(GEN_DELAY);
            for (size_t x = 1; x < GRIDX-1; x++) {
                for (size_t y = 1; y < GRIDY-1; y++) {
                    grid[x][y] = newgrid[x][y];
                }
            }
        }
    }

    void drawGrid(TFT_eSPI& tft, uint8_t** grid, uint8_t** newgrid) {
    uint16_t color = TFT_WHITE;
    for (int16_t x = 1; x < GRIDX - 1; x++) {
        for (int16_t y = 1; y < GRIDY - 1; y++) {
        if ((grid[x][y]) != (newgrid[x][y])) {
            if (newgrid[x][y] == 1) color = 0xFFFF; //random(0xFFFF);
            else color = 0;
            tft.fillRect(CELLXY * x, CELLXY * y, CELLXY, CELLXY, color);
        }
        }
    }
    }

    void initGrid(uint8_t** grid, uint8_t** newgrid) {
    for (int16_t x = 0; x < GRIDX; x++) {
        for (int16_t y = 0; y < GRIDY; y++) {
        newgrid[x][y] = 0;

        if (x == 0 || x == GRIDX - 1 || y == 0 || y == GRIDY - 1) {
            grid[x][y] = 0;
        }
        else {
            if (random(3) == 1)
            grid[x][y] = 1;
            else
            grid[x][y] = 0;
        }
        }
    }
    }

    void computeCA(uint8_t** grid, uint8_t** newgrid) {
    for (int16_t x = 1; x < GRIDX; x++) {
        for (int16_t y = 1; y < GRIDY; y++) {
        int neighbors = getNumberOfNeighbors(x, y, grid);
        if (grid[x][y] == 1 && (neighbors == 2 || neighbors == 3 )){
            newgrid[x][y] = 1;
        }else if (grid[x][y] == 1)  newgrid[x][y] = 0;
        if (grid[x][y] == 0 && (neighbors == 3)) {
            newgrid[x][y] = 1;
        } else if (grid[x][y] == 0) newgrid[x][y] = 0;
        }
    }
    }

    // Check the Moore neighborhood
    int getNumberOfNeighbors(int x, int y, const uint8_t** grid) {
    return grid[x - 1][y] + grid[x - 1][y - 1] + grid[x][y - 1] + grid[x + 1][y - 1] + grid[x + 1][y] + grid[x + 1][y + 1] + grid[x][y + 1] + grid[x - 1][y + 1];
    }
}