#include <string.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <fstream>

#include "milp_solver_interface.h"
#include "fpga.h"
#include "flora.h"
#include "fine_grained.h"

#define MAX_RECONF_MODULES 1
#define MAX_MODULES 100

//TODO:This should be done in a better way
#define CLB_MARGIN 30
#define BRAM_MARGIN 10
#define DSP_MARGIN 0

namespace fs = std::experimental::filesystem;
using namespace std;
//using namespace Ui;

//A class to contain info about a reconfigurable module
typedef struct{
    std::string rm_tag;
    std::string source_path;
    std::string top_module;
}reconfigurable_module;

typedef struct{
    unsigned long num_rm_modules;
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
    std::string Project_dir = "/home/biruk/test_pr_dir";
    std::string Src_path = Project_dir + "/Sources";
    std::string hdl_copy_path = Src_path + "/hdl";
    std::string fplan_xdc_file = Src_path + "/constraints/pblocks.xdc";
    std::string tcl_project = Project_dir + "/Tcl";
    std::string synthesis_script = Project_dir + "/ooc_synth.tcl" ;
    std::string impl_script = Project_dir + "/impl.tcl";;
    //pointer to an instance of flora
    flora *fl_inst;
    input_to_flora in_flora;

#ifdef WITH_PARTITIONING
    vector<double> slacks ;
    vector <double> HW_WCET;
#endif


    void prep_input();
    void prep_proj_directory();
    void generate_synthesis_tcl();
    void start_synthesis(std::string synth_script);
    void parse_synthesis_report();
    void generate_impl_tcl(flora *fl);
    void start_implementation(std::string impl_script); 
    
    explicit pr_tool(input_to_pr *);

    ~pr_tool();
};
