#include <string.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <fstream>

#include "zynq_model.h"
#include "fpga.h"
#include "flora.h"
#include "fine_grained.h"

#define MAX_RECONF_MODULES 1
#define MAX_MODULES 100

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
    std::string Project_dir = "/home/holmes/test_pr_dir";
    std::string Src_path = Project_dir + "/Sources";
    std::string hdl_copy_path = Src_path + "/hdl";
    std::string tcl_project = Project_dir + "/Tcl";
    std::string synthesis_script;

    //pointer to an instance of flora
    flora *fl_inst;

#ifdef WITH_PARTITIONING
    vector<double> slacks ;
    vector <double> HW_WCET;
#endif


    void prep_input();
    void prep_proj_directory();
    void generate_synthesis_tcl();
    void start_synthesis(std::string synth_script);
    void parse_synthesis_report();

    explicit pr_tool(input_to_pr *);

    ~pr_tool();
};
