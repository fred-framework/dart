#ifndef FP_H
#define FP_H

/*
#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QString>
*/

#include "gurobi_c++.h"
#include "zynq_model.h"
#include "fpga.h"
#include "csv_data_manipulator.hpp"
#include <string>

namespace Ui {
class fp;
}

enum fpga_type {
    ZYNQ = 0,
    VIRTEX,
    VIRTEX_5,
    PYNQ
};

typedef std::vector<pos> position_vec;
typedef std::vector<std::vector<unsigned long>> vec_2d;
typedef struct{
    unsigned long clb;
    unsigned long bram;
    unsigned long dsp;
}slot;

typedef struct{
    unsigned long num_slots;
    fpga_type type_of_fpga;
    std::string path_to_input;
}input_to_flora;

#define MY_RAND() ((double)((double)rand()/(double)RAND_MAX))

class flora //: public QDialog
{
    //Q_OBJECT

public:
    explicit flora(input_to_flora*);
    ~flora();

    zynq_7010 *zynq;

/*
 *  virtex *virt;
    virtex_5 *virt_5;
    pynq *pynq_inst;
 */ 
    param_to_solver param;
    input_to_flora *flora_input;

    unsigned long num_slots = 0;
    enum fpga_type type = ZYNQ;
    unsigned long connections;

    std::vector<unsigned long> clb_vector =  std::vector<unsigned long>(MAX_SLOTS);
    std::vector<unsigned long> bram_vector = std::vector<unsigned long>(MAX_SLOTS);
    std::vector<unsigned long> dsp_vector =  std::vector<unsigned long>(MAX_SLOTS);

    std::vector<int> clb_from_solver  = std::vector<int>(MAX_SLOTS);
    std::vector<int> bram_from_solver = std::vector<int>(MAX_SLOTS);
    std::vector<int> dsp_from_solver  = std::vector<int>(MAX_SLOTS);

    std::vector<slot> sl_array = std::vector<slot>(MAX_SLOTS);

    vec_2d connection_matrix = std::vector<std::vector<unsigned long>> (MAX_SLOTS, std::vector<unsigned long> (MAX_SLOTS, 0));

    std::vector<int> eng_x =  std::vector<int>(MAX_SLOTS);
    std::vector<int> eng_y = std::vector<int>(MAX_SLOTS);
    std::vector<int> eng_w =  std::vector<int>(MAX_SLOTS);
    std::vector<int> eng_h =  std::vector<int>(MAX_SLOTS);

    std::vector<int> x_vector =  std::vector<int>(MAX_SLOTS);
    std::vector<int> y_vector =  std::vector<int>(MAX_SLOTS);
    std::vector<int> w_vector =  std::vector<int>(MAX_SLOTS);
    std::vector<int> h_vector =  std::vector<int>(MAX_SLOTS);

    position_vec forbidden_region = position_vec(MAX_SLOTS);
//    position_vec forbidden_region_pynq = position_vec(MAX_SLOTS);
//    position_vec forbidden_region_virtex = position_vec(MAX_SLOTS);
//    position_vec forbidden_region_virtex_5 = position_vec(MAX_SLOTS);
    std::vector<unsigned long> get_units_per_task(unsigned long n,
                                                  unsigned long n_units,
                                                  unsigned long n_min,
                                                  unsigned long n_max);

    param_from_solver from_solver = {&eng_x, &eng_y,
                                    &eng_w, &eng_h,
                                    &clb_from_solver,
                                    &bram_from_solver,
                                    &dsp_from_solver};

    std::vector<std::string> cell_name = std::vector<std::string>(MAX_SLOTS);

    void clear_vectors();
    void prep_input();
    void start_optimizer();
    void generate_xdc();

    void init_fpga(enum fpga_type);
    void init_gui();
    void plot_rects(param_from_solver *);
    bool is_compatible(std::vector<slot> ptr, unsigned long slot_num, int max, unsigned long min, int type);

};

#endif // FP_H
