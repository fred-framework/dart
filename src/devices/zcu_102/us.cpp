#include "ultrascale.h"

ultrascale::ultrascale()
{
    ultrascale::initialize_clk_reg();
}

void ultrascale::initialize_clk_reg()
{
    unsigned int i = 0;

    for(i = 0; i < US_CLK_REG ;  i++) {
    init_clk_reg(i, clk_reg_pos[i], clb_per_col,
                bram_per_col, dsp_per_col,
                bram_in_reg[i], dsp_in_reg[i],
                bram_pos[i], dsp_pos[i]);
    }
}


