#include "flora.h"
#include "fpga.h"
#include <iostream>
#include <string>

using namespace std;

flora::flora(input_to_flora *input_fl)
{

    flora_input = input_fl;
    
    cout << "num of slots **** " << flora_input->num_slots <<endl;
    cout << "type of FPGA **** " << flora_input->type_of_fpga <<endl;
    cout << "path for input **** " << flora_input->path_to_input <<endl;
    
    if(flora_input->num_slots > 0) {
        num_slots = flora_input->num_slots;
        type = flora_input->type_of_fpga; 
    }
    else {
        cout <<"Please a number of slots > 0";
        exit(-1);
    }
    
    

}

flora::~flora()
{
    cout << "destruction " << endl;
}

//Prepare the input
void flora::clear_vectors()
{
    clb_vector.clear();
    bram_vector.clear();
    dsp_vector.clear();

    eng_x.clear();
    eng_y.clear();
    eng_w.clear();
    eng_h.clear();

    x_vector.clear();
    y_vector.clear();
    w_vector.clear();
    h_vector.clear();



}

void flora::prep_input()
{
    unsigned long row, col;
    int i , k;
    unsigned int ptr;
    string str;
    CSVData csv_data(flora_input->path_to_input);

    row = csv_data.rows();
    col = csv_data.columns();

    cout << endl << "resource requirement of the input slots " <<endl;
    cout << "\t clb " << " \t bram " << "\t dsp " <<endl;
    for(i = 0, ptr = 0, k = 0; i < num_slots; i++, ptr++) {
       cout << "slot " << i;  
        str = csv_data.get_value(i, k++);
        clb_vector[ptr] = std::stoi(str);

        str = csv_data.get_value(i, k++);
        bram_vector[ptr] = std::stoi(str);

        str = csv_data.get_value(i, k++);
        dsp_vector[ptr] = std::stoi(str);
        k = 0;
        cout << "\t " << clb_vector[ptr] << "\t " << bram_vector[ptr] << "\t " 
             << dsp_vector[ptr] << endl;
      }
    }


void flora::start_optimizer()
{
    int i;
    param.bram = &bram_vector;
    param.clb  = &clb_vector;
    param.dsp  = &dsp_vector;
    param.num_slots = num_slots;
    param.num_connected_slots = connections;
    param.conn_vector = &connection_matrix;

    if(type ==ZYNQ){
        zynq = new zynq_7010();
        for(i = 0; i < zynq->num_forbidden_slots; i++) {
            forbidden_region[i] = zynq->forbidden_pos[i];
        //cout<< " fbdn" << forbidden_region[i].x << endl;
    }

        param.num_forbidden_slots = zynq->num_forbidden_slots;
        param.num_rows = zynq->num_rows;
        param.width = zynq->width;
        param.fbdn_slot = &forbidden_region;
        param.num_clk_regs  = zynq->num_clk_reg /2;
        param.clb_per_tile  = ZYNQ_CLB_PER_TILE;
        param.bram_per_tile = ZYNQ_BRAM_PER_TILE;
        param.dsp_per_tile  = ZYNQ_DSP_PER_TILE;

        zynq_start_optimizer(&param, &from_solver);
    }

}
//call milp solver



//call generate xdc



//print outputs


