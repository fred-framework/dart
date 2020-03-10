#ifndef ZYNQ_H
#define ZYNQ_H

#define MAX_SLOTS 100
#define CLB  0
#define BRAM 1
#define DSP  2

//parameters for zynq
#define ZYNQ_CLB_TOT 2200
#define ZYNQ_BRAM_TOT 60
#define ZYNQ_DSP_TOT 80
#define ZYNQ_CLB_MIN 10
#define ZYNQ_BRAM_MIN 0
#define ZYNQ_DSP_MIN 0
#define ZYNQ_FORBIDDEN 2
#define ZYNQ_NUM_ROWS 10
#define ZYNQ_WIDTH 29
#define ZYNQ_CLK_REG 4
#define ZYNQ_CLK00_BRAM 1
#define ZYNQ_CLK01_BRAM 1
#define ZYNQ_CLK10_BRAM 2
#define ZYNQ_CLK11_BRAM 2
#define ZYNQ_CLK00_DSP 1
#define ZYNQ_CLK01_DSP 1
#define ZYNQ_CLK10_DSP 1 
#define ZYNQ_CLK11_DSP 1
#define ZYNQ_CLB_PER_TILE 50
#define ZYNQ_BRAM_PER_TILE 10
#define ZYNQ_DSP_PER_TILE 20

//parameters for virtex_5
#define VIRTEX_5_CLB_TOT 8140
#define VIRTEX_5_BRAM_TOT 160
#define VIRTEX_5_DSP_TOT 64
#define VIRTEX_5_CLB_MIN 10
#define VIRTEX_5_BRAM_MIN 0
#define VIRTEX_5_DSP_MIN 0
#define VIRTEX_5_CLK_REG 16
#define VIRTEX_5_FORBIDDEN 2
#define VIRTEX_5_NUM_ROWS 4
#define VIRTEX_5_WIDTH 62
#define VIRTEX_5_CLB_PER_TILE 20
#define VIRTEX_5_BRAM_PER_TILE 4
#define VIRTEX_5_DSP_PER_TILE 8
#define VIRTEX_5_CLK00_BRAM 2
#define VIRTEX_5_CLK01_BRAM 2
#define VIRTEX_5_CLK02_BRAM 2
#define VIRTEX_5_CLK03_BRAM 2
#define VIRTEX_5_CLK04_BRAM 2
#define VIRTEX_5_CLK05_BRAM 2
#define VIRTEX_5_CLK06_BRAM 2
#define VIRTEX_5_CLK07_BRAM 2
#define VIRTEX_5_CLK10_BRAM 3
#define VIRTEX_5_CLK11_BRAM 3
#define VIRTEX_5_CLK12_BRAM 3
#define VIRTEX_5_CLK13_BRAM 3
#define VIRTEX_5_CLK14_BRAM 3
#define VIRTEX_5_CLK15_BRAM 3
#define VIRTEX_5_CLK16_BRAM 3
#define VIRTEX_5_CLK17_BRAM 3
#define VIRTEX_5_CLK00_DSP 1
#define VIRTEX_5_CLK01_DSP 1
#define VIRTEX_5_CLK02_DSP 1
#define VIRTEX_5_CLK03_DSP 1
#define VIRTEX_5_CLK04_DSP 1
#define VIRTEX_5_CLK05_DSP 1
#define VIRTEX_5_CLK06_DSP 1
#define VIRTEX_5_CLK07_DSP 1
#define VIRTEX_5_CLK10_DSP 0
#define VIRTEX_5_CLK11_DSP 0
#define VIRTEX_5_CLK12_DSP 0
#define VIRTEX_5_CLK13_DSP 0
#define VIRTEX_5_CLK14_DSP 0
#define VIRTEX_5_CLK15_DSP 0
#define VIRTEX_5_CLK16_DSP 0
#define VIRTEX_5_CLK17_DSP 0

//parameters for virtex_7
#define VIRTEX_CLB_TOT 21487
#define VIRTEX_BRAM_TOT 500
#define VIRTEX_DSP_TOT 900
#define VIRTEX_CLB_MIN 10
#define VIRTEX_BRAM_MIN 0
#define VIRTEX_DSP_MIN 0
#define VIRTEX_FORBIDDEN 4
#define VIRTEX_NUM_ROWS 10
#define VIRTEX_WIDTH 103
#define VIRTEX_CLK_REG 14
#define VIRTEX_CLB_PER_TILE 50
#define VIRTEX_BRAM_PER_TILE 10
#define VIRTEX_DSP_PER_TILE 20
//bram descritpion on virtex
#define VIRTEX_CLK00_BRAM 4
#define VIRTEX_CLK01_BRAM 4
#define VIRTEX_CLK02_BRAM 4
#define VIRTEX_CLK03_BRAM 4
#define VIRTEX_CLK04_BRAM 4
#define VIRTEX_CLK05_BRAM 2
#define VIRTEX_CLK06_BRAM 2
#define VIRTEX_CLK10_BRAM 4
#define VIRTEX_CLK11_BRAM 4
#define VIRTEX_CLK12_BRAM 4
#define VIRTEX_CLK13_BRAM 4
#define VIRTEX_CLK14_BRAM 5
#define VIRTEX_CLK15_BRAM 5
#define VIRTEX_CLK16_BRAM 5
#define VIRTEX_CLK00_DSP 4
#define VIRTEX_CLK01_DSP 4
#define VIRTEX_CLK02_DSP 4
#define VIRTEX_CLK03_DSP 4
#define VIRTEX_CLK04_DSP 4
#define VIRTEX_CLK05_DSP 2
#define VIRTEX_CLK06_DSP 2
#define VIRTEX_CLK10_DSP 3
#define VIRTEX_CLK11_DSP 3
#define VIRTEX_CLK12_DSP 3
#define VIRTEX_CLK13_DSP 3
#define VIRTEX_CLK14_DSP 3
#define VIRTEX_CLK15_DSP 3
#define VIRTEX_CLK16_DSP 3

//parameters for pynq
#define PYNQ_CLK_REG 6
#define PYNQ_WIDTH 70
#define PYNQ_NUM_ROWS 10
#define PYNQ_FORBIDDEN 3
#define PYNQ_CLB_PER_TILE 50
#define PYNQ_BRAM_PER_TILE 10
#define PYNQ_DSP_PER_TILE 20
typedef struct {
    int x;
    int y;
    int w;
    int h;
}pos;

typedef struct {
    pos clk_reg_pos;
    int clb_per_column;
    int bram_per_column;
    int dsp_per_column;
    int clb_num;
    int bram_num;
    int dsp_num;
    int forbidden_num;
    int *bram_pos;
    int *dsp_pos;
}fpga_clk_reg;

#define init_clk_reg(id, pos, clb, bram, dsp, num_bram, num_dsp,\
                               pos_bram, pos_dsp) \
                               clk_reg[id].clk_reg_pos = pos;    \
                               clk_reg[id].clb_per_column = clb;\
                               clk_reg[id].bram_per_column = bram; \
                               clk_reg[id].dsp_per_column = dsp;  \
                               clk_reg[id].bram_num = num_bram;  \
                               clk_reg[id].dsp_num = num_dsp;\
                               clk_reg[id].bram_pos = pos_bram;\
                               clk_reg[id].dsp_pos = pos_dsp;


class zynq_7010
{
public:
    int clb_per_col  = ZYNQ_CLB_PER_TILE;
    int bram_per_col = ZYNQ_BRAM_PER_TILE;
    int dsp_per_col  = ZYNQ_DSP_PER_TILE;
    int num_clk_reg =  ZYNQ_CLK_REG;

    fpga_clk_reg clk_reg[ZYNQ_CLK_REG];
    pos clk_reg_pos [ZYNQ_CLK_REG] = {{0,  0,  14, 50},
                                      {0,  50, 14, 50},
                                      {15, 0,  14, 50},
                                      {15, 50, 14, 50}};

    unsigned long num_rows = ZYNQ_NUM_ROWS;
    unsigned long width    = ZYNQ_WIDTH;

    int bram_in_reg[ZYNQ_CLK_REG] =  {1, 1, 2, 2};
    int bram_pos[ZYNQ_CLK_REG][10] = {{4, 0, 0},  {4, 0, 0},
                                     {18, 25, 0}, {18, 25, 0}};

    int dsp_in_reg[ZYNQ_CLK_REG] = {1, 1, 1, 1};
    int dsp_pos[ZYNQ_CLK_REG][3] = {{7, 0, 0},
                                    {7, 0, 0},
                                    {22, 0, 0},
                                    {22, 0, 0}};

    unsigned long num_forbidden_slots = ZYNQ_FORBIDDEN;
    pos forbidden_pos[ZYNQ_FORBIDDEN] = {{9, 0, 1, 20},
                                        {14, 0, 1, 20}};

    void initialize_clk_reg();
    zynq_7010();
};

class virtex
{
public:
    unsigned long num_clk_reg = VIRTEX_CLK_REG;
    fpga_clk_reg clk_reg[VIRTEX_CLK_REG];
    pos clk_reg_pos [VIRTEX_CLK_REG]= {{0,  300, 54, 50},
                                       {0,  250, 54, 50},
                                       {0,  200, 54, 50},
                                       {0,  150, 54, 50},
                                       {0,  100, 54, 50},
                                       {0,  50,  54, 50},
                                       {0,  0,   54, 50},
                                       {55, 300, 54, 55},
                                       {55, 250, 54, 55},
                                       {55, 200, 54, 55},
                                       {55, 150, 54, 55},
                                       {55, 100, 54, 55},
                                       {55, 50,  54, 55},
                                       {55, 0,   54, 55}};

    int clb_per_col  = VIRTEX_CLB_PER_TILE;
    int bram_per_col = VIRTEX_BRAM_PER_TILE;
    int dsp_per_col  = VIRTEX_DSP_PER_TILE;

    unsigned long num_rows = VIRTEX_NUM_ROWS;
    unsigned long width = VIRTEX_WIDTH;

    int bram_in_reg[VIRTEX_CLK_REG] = {4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4, 5, 5, 5};
    int bram_pos[VIRTEX_CLK_REG][5] = {{4, 15, 20, 35}, {4, 15, 20, 35}, {4, 15, 20, 35},
                                        {4, 15, 20, 35}, {4, 15, 20, 35},
                                        {20, 35}, {20, 35}, {56, 73, 86, 92},
                                        {56, 73, 86, 92}, {56, 73, 86, 92},
                                        {56, 73, 86, 92}, {56, 73, 86, 92, 99},
                                        {56, 73, 86, 92, 99}, {56, 73, 86, 92, 99}
                                      };

    int dsp_in_reg[VIRTEX_CLK_REG] = {4, 4, 4, 4, 4, 2, 2, 3, 3, 3, 3, 3, 3, 3};
    int dsp_pos[VIRTEX_CLK_REG][5] = {{7, 12, 23, 32}, {7, 12, 23, 32}, {7, 12, 23, 32},
                                      {7, 12, 23, 32}, {7, 12, 23, 32}, {23, 32},
                                      {23, 32}, {59, 83, 95}, {59, 83, 95},
                                      {59, 83, 95}, {59, 83, 95}, {59, 83, 95},
                                      {59, 83, 95}, {59, 83, 95}};

    unsigned long num_forbidden_slots = VIRTEX_FORBIDDEN;
    pos forbidden_pos[VIRTEX_FORBIDDEN] = {{0,   50,  18, 20},
                                           {74,  50,  8,  20},
                                           {89,  30,  4,  10},
                                           {104, 0,   5,  40}};

    void initialize_clk_reg();
    virtex();
};


class virtex_5
{
public:
    int num_clk_reg = VIRTEX_5_CLK_REG;
    fpga_clk_reg clk_reg[VIRTEX_5_CLK_REG];
    pos clk_reg_pos [VIRTEX_5_CLK_REG] ={{0,  140, 27, 20},
                                         {0,  120, 27, 20},
                                         {0,  100, 27, 20},
                                         {0,  80, 27, 20},
                                         {0,  60, 27, 20},
                                         {0,  40, 27, 20},
                                         {0,  20,  27, 20},
                                         {0,  0,  27, 20},
                                         {28, 140, 34, 20},
                                         {28, 120, 34, 20},
                                         {28, 100, 34, 20},
                                         {28, 80, 34, 20},
                                         {28, 60, 34, 20},
                                         {28, 40, 34, 20},
                                         {28, 20, 34, 20},
                                         {28, 0, 34, 20}};


    int clb_per_col  =  VIRTEX_5_CLB_PER_TILE;
    int bram_per_col =  VIRTEX_5_BRAM_PER_TILE;
    int dsp_per_col  =  VIRTEX_5_DSP_PER_TILE;

    unsigned long num_rows = VIRTEX_5_NUM_ROWS;
    unsigned long width = VIRTEX_5_WIDTH;

    int bram_in_reg[VIRTEX_5_CLK_REG] = {2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};

    int bram_pos[VIRTEX_5_CLK_REG][5] = {{4, 15}, {4, 15}, {4, 15}, {4, 15}, {4, 15},
                                         {4, 15}, {4, 15}, {4, 15}, {40, 51, 62},
                                         {40, 51, 62}, {40, 51, 62}, {40, 51, 62},
                                         {40, 51, 62}, {40, 51, 62}, {40, 51, 62},
                                         {40, 51, 62}};

    int dsp_in_reg[VIRTEX_5_CLK_REG] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

    int dsp_pos[VIRTEX_5_CLK_REG][5] = {{18}, {18}, {18}, {18},{18} ,{18}, {18}, {18},
                                        };

    unsigned long num_forbidden_slots = VIRTEX_5_FORBIDDEN;
    pos forbidden_pos[VIRTEX_5_FORBIDDEN] = {{56, 0,  1, 80},
                                             {61, 20, 1, 30}};

    void initialize_clk_reg();
    virtex_5();

};

class pynq
{
public:
    int num_clk_reg = PYNQ_CLK_REG;
    fpga_clk_reg clk_reg[PYNQ_CLK_REG];
    pos clk_reg_pos [PYNQ_CLK_REG] = {{0,  0,  31, 50},
                                      {0,  50, 31, 50},
                                      {0,  100, 31, 50},
                                      {32, 0,  38, 50},
                                      {32, 50, 38, 50},
                                      {32, 100, 38, 50}};

    int clb_per_col  = PYNQ_CLB_PER_TILE;
    int bram_per_col = PYNQ_BRAM_PER_TILE;
    int dsp_per_col  = PYNQ_DSP_PER_TILE;

    unsigned long num_rows = PYNQ_NUM_ROWS;
    unsigned long width    = PYNQ_WIDTH;

    int bram_in_reg[PYNQ_CLK_REG]  = {1, 1, 3, 3, 3, 3};
    int bram_pos[PYNQ_CLK_REG][10] = {{21, 0, 0}, {21, 0, 0}, {5, 16, 21},
                                     {35, 55, 66}, {35, 55, 66}, {35, 55, 66}};

    int dsp_in_reg[PYNQ_CLK_REG] = {1, 1, 3, 2, 2, 2};
    int dsp_pos[PYNQ_CLK_REG][3] = {{24, 0, 0},
                                    {24, 0, 0},
                                    {8, 13, 24},
                                    {58, 63, 0},
                                    {58, 63, 0},
                                    {58, 63, 0}};

    unsigned long num_forbidden_slots = PYNQ_FORBIDDEN;
    pos forbidden_pos[PYNQ_FORBIDDEN] = {{0, 10, 17, 20},
                                        {42, 10, 7,  20},
                                        {47, 0,  2,  10}};

    void initialize_clk_reg();
    pynq();
};
#endif
