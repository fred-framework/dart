#include <string.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "zynq_model.h"
#include "fpga.h"
#include "flora.h"
#include "fine_grained.h"

#define MAX_RECONF_MODULES 1
#define MAX_MODULES 100
/*
typedef std::vector<pos> position_vec;
typedef std::vector<std::vector<unsigned long>> vec_2d;

typedef struct{
    unsigned long clb;
    unsigned long bram;
    unsigned long dsp;
}slot;
*/
using namespace std;
//using namespace Ui;

//A class to contain info about a reconfigurable module
typedef struct{
    std::string source_path;
    std::string top_module;
}reconfigurable_module;

typedef struct{
    unsigned long num_rm_modules;
    fpga_type type_of_fpga;
    std::string path_to_input;
}input_to_pr;

//The main class to process everything
class pr_tool
{

public:

    //Reconfigurable module instance
    vector<reconfigurable_module> rm_list;
    unsigned long num_rm_modules;
    input_to_pr *input_pr;
    fpga_type type;

    //variables to manage project directory
    std::string project_dir = "/home/sholmes/test1";
    std::string source_path = project_dir + "/Sources/";
    std::string hdl_copy_path = source_path + "/hdl/";
    std::string tcl_project = project_dir + "/Tcl/";

/*
    //variables and methods for process management
    param_to_solver to_solver;
    static vector <std::string> file_path;
    void update_file_path(std::string filepath);
    void parse_synthesis_report();

    //variables and methods related to FPGA
    fpga_zynq *zynq;
    position_vec forbidden_region_zynq = position_vec(MAX_SLOTS);
    void paint_zynq();

    //variables related to FPGA graphics
    unsigned long clb_width = 10;
    unsigned long clb_height = 8, bram_height = 40, dsp_height = 20;
    unsigned long total_height;
    static enum fpga_type type;
    std::vector<unsigned long> x_vector =  std::vector<unsigned long>(MAX_SLOTS);
    std::vector<unsigned long> y_vector =  std::vector<unsigned long>(MAX_SLOTS);
    std::vector<unsigned long> w_vector =  std::vector<unsigned long>(MAX_SLOTS);
    std::vector<unsigned long> h_vector =  std::vector<unsigned long>(MAX_SLOTS);


    //variables to pass to MILP optimizer
    Taskset *task_set;
    Platform *platform;
*/
    static vector<double> slacks ;
    static vector <double> HW_WCET;
/*  
    vector<unsigned long> eng_x = vector<unsigned long>(MAX_SLOTS);
    vector<unsigned long> eng_y = vector<unsigned long>(MAX_SLOTS);
    vector<unsigned long> eng_w = vector<unsigned long>(MAX_SLOTS);
    vector<unsigned long> eng_h = vector<unsigned long>(MAX_SLOTS);
    vec_2d connection_matrix =    vector<vector<unsigned long>>
                                        (MAX_SLOTS, vector<unsigned long>
                                         (MAX_SLOTS, 0));
    vec_2d modules_in_partition = vector<vector<unsigned long>> (MAX_SLOTS, vector<unsigned long> (MAX_SLOTS, 0));

    Vec modules_per_partition =vector<unsigned long>(MAX_SLOTS, 0);

    param_from_solver from_solver = {0, 0, &modules_in_partition, &modules_per_partition, &eng_x, &eng_y, &eng_w, &eng_h};

    */
    void prep_input();
    void prep_proj_directory();
/*
    //Variables to generate xdc
    string floorplan_addr;
    generate_fp_zynq *generate_xdc;

    void init_solver_params();
    void start_optimizer();
    void plot_rects(param_from_solver *fs);
    void generate_impl_script(param_from_solver *fs);
*/
    explicit pr_tool(input_to_pr *);

    ~pr_tool();

};
