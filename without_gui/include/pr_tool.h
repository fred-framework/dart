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
#ifdef FPGA_PYNQ
#define CLB_MARGIN  150
#define BRAM_MARGIN 10
#define DSP_MARGIN  10
#elif FPGA_ZYNQ
#define CLB_MARGIN 30
#define BRAM_MARGIN 0
#define DSP_MARGIN  0
#else
#define CLB_MARGIN  30
#define BRAM_MARGIN  0
#define DSP_MARGIN   0
#endif

#ifdef __cpp_lib_filesystem
    #include <filesystem>
    using fs = std::filesystem;
#elif __cpp_lib_experimental_filesystem
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #error "no filesystem support ='("
#endif

using namespace std;
//using namespace Ui;

//A class to contain info about a reconfigurable module
typedef struct{
#ifndef WITH_PARTITIONING
    unsigned int partition_id;
#endif
    std::string rm_tag;
    //std::string source_path;
    std::string top_module;
}reconfigurable_module;

typedef struct{
#ifdef WITH_PARTITIONING
    unsigned long num_rm_modules;
#else
    unsigned long num_rm_partitions;
#endif
    std::string static_dcp_file;
    std::string static_top_module;
    std::string path_to_input;
    std::string path_to_output;
}input_to_pr;

typedef struct {
    unsigned int num_modules_in_partition = 0;
    unsigned int num_hw_tasks_in_part = 0;
    std::vector<unsigned int> rm_id;
}partition_allocation;

//The main class to process everything
class pr_tool
{

// get the directory separator depending on the OS
string dir_separator(){
    string s;
    s.assign(1,fs::path::preferred_separator);    
    return s;
}

// split a string, or a path, by the delimiter
vector<string> split(const string& text, char delimiter) {
    string tmp;
    vector<string> stk;
    stringstream ss(text);
    while(getline(ss,tmp, delimiter)) {
        stk.push_back(tmp);
    }
    return stk;
}


public:

    //Reconfigurable module instance
    vector<reconfigurable_module> rm_list;
#ifdef WITH_PARTITIONING
    unsigned long num_rm_modules;
#else
    unsigned long num_rm_partitions;
    unsigned long num_rm_modules = 0;
#endif 
    input_to_pr *input_pr;
    fpga_type type;
    string dart_path;

    //variables to manage project directory
    std::string Project_dir; //= "/home/holmes/test_pr_dir";
    std::string Src_path;// = Project_dir + "/Sources";
    std::string hdl_copy_path; //= Src_path + "/hdl";
    std::string fplan_xdc_file;// = Src_path + "/constraints/pblocks.xdc";
    std::string tcl_project; // = Project_dir + "/Tcl";
    std::string synthesis_script; // = Project_dir + "/ooc_synth.tcl" ;
    std::string impl_script; // = Project_dir + "/impl.tcl";
    std::string fred_dir;
    //pointer to an instance of flora
    flora *fl_inst;
    input_to_flora in_flora;

#ifdef WITH_PARTITIONING
    vector<double> slacks ;
    vector <double> HW_WCET;
#else
    std::vector<partition_allocation> alloc =  std::vector<partition_allocation>(MAX_SLOTS);
    unsigned int max_modules_in_partition = 0;
#endif


    void prep_input();
    void init_dir_struct();
    void prep_proj_directory();
    void create_vivado_project();
    void generate_synthesis_tcl();
    void run_vivado(std::string synth_script);
    void parse_synthesis_report();
    void generate_impl_tcl(flora *fl);
    void generate_fred_files(flora *fptr);

    explicit pr_tool(input_to_pr *);

    ~pr_tool();
};
