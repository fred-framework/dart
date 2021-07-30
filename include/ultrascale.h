#include "fpga.h"

//parameters for ultrascale
#define US_CLK_REG 14
#define US_WIDTH 130
#define US_NUM_ROWS 30
#define US_FORBIDDEN 1
#define US_CLB_PER_TILE 60
#define US_BRAM_PER_TILE 12
#define US_DSP_PER_TILE 24
#define US_CLB_TOT  34260 
#define US_BRAM_TOT 912
#define US_DSP_TOT  2520


class ultrascale
{
public:
    int num_clk_reg = US_CLK_REG;
    fpga_clk_reg clk_reg[US_CLK_REG];
    
    pos clk_reg_pos [US_CLK_REG] = {{0,  0,  31, 50},
                                      {0,  50, 31, 50},
                                      {0,  100, 31, 50},
                                      {32, 0,  38, 50},
                                      {32, 50, 38, 50},
                                      {32, 100, 38, 50}};
    
    int clb_per_col  = US_CLB_PER_TILE;
    int bram_per_col = US_BRAM_PER_TILE;
    int dsp_per_col  = US_DSP_PER_TILE;

    unsigned long num_rows = US_NUM_ROWS;
    unsigned long width    = US_WIDTH;

    int bram_in_reg[US_CLK_REG]  = {1, 1, 3, 3, 3, 3};
    int bram_pos[US_CLK_REG][10] = {{21, 0, 0}, {21, 0, 0}, {5, 16, 21},
                                     {35, 55, 66}, {35, 55, 66}, {35, 55, 66}};

    int dsp_in_reg[US_CLK_REG] = {1, 1, 3, 2, 2, 2};
    int dsp_pos[US_CLK_REG][3] = {{24, 0, 0},
                                    {24, 0, 0},
                                    {8, 13, 24},
                                    {58, 63, 0},
                                    {58, 63, 0},
                                    {58, 63, 0}};

    unsigned long num_forbidden_slots = US_FORBIDDEN;

    pos forbidden_pos[US_FORBIDDEN] = {{0, 0, 35, 30}};


    void initialize_clk_reg();
    ultrascale();
};
