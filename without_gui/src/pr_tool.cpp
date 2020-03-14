#include <iostream>
#include "pr_tool.h"

using namespace std;

pr_tool::pr_tool(input_to_pr *pr_input) 
{
    input_pr = pr_input; 
    cout << " Begin PR_tool " <<endl;

    if(input_pr->num_rm_modules > 0){
        num_rm_modules = pr_input->num_rm_modules;
        type = pr_input->type_of_fpga;

        cout << "num of slots **** " << num_rm_modules <<endl;
        cout << "type of FPGA **** " << type <<endl;
        cout << "path for input **** " << pr_input->path_to_input <<endl;
        
        prep_input(); 
        prep_proj_directory();
    }
    else {
        cout <<"The number of Reconfigurable modules > 0";
        exit(-1);
    }
    
}

pr_tool::~pr_tool()
{
    cout << " destruction of PR_tool" <<endl;
}

void pr_tool::prep_input()
{
    unsigned long row, col;
    int i , k;
    unsigned int ptr;
    reconfigurable_module rm;

    CSVData csv_data(input_pr->path_to_input);

    row = csv_data.rows();
    col = csv_data.columns();

    cout << endl << "reading inputs " <<endl;
    for(i = 0, ptr = 0, k = 0; i < num_rm_modules; i++, ptr++) {

#ifdef WITH_PARTITIONING
        str = csv_data.get_value(i, k++);
        HW_WCET[ptr] = std::stod(str);

        str = csv_data.get_value(i, k++);
        slacks[ptr] = std::stod(str);
#endif
        rm.source_path = csv_data.get_value(i, k++);
        rm.top_module = csv_data.get_value(i, k++);
        rm_list.push_back(rm);

        k = 0;
    }
}

void pr_tool::prep_proj_directory()
{
    int status;
    //TODO: check if directory exists
    status = mkdir(project_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(source_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/prj").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/xdc").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/cores").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/netlist").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Synth").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Implement").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Checkpoint").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Bitstreams").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(hdl_copy_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(tcl_project.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    //copy source files of each RM to Source/core/top_name
      

    //create the .prj file using the python script
}

