#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "pr_tool.h"
#include <fstream>

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
        
        //Instantiate flora
        //flora fl_inst;
 
        prep_input(); 
        //prep_proj_directory();
        //generate_synthesis_tcl();
        //start_synthesis(synthesis_script);
        parse_synthesis_report();
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
    std::string str;
    unsigned long row, col;
    int i , k;
    unsigned int ptr;
    reconfigurable_module rm;

    CSVData csv_data(input_pr->path_to_input);

    row = csv_data.rows();
    col = csv_data.columns();

    cout << endl << "reading inputs " << row << " " << col <<endl;
    for(i = 0, ptr = 0, k = 0; i < num_rm_modules; i++, ptr++) {

#ifdef WITH_PARTITIONING
        str = csv_data.get_value(i, k++);
        HW_WCET.push_back(std::stod(str));
cout <<"here"<<endl;
        str = csv_data.get_value(i, k++);
        slacks.push_back(std::stod(str));
#endif
        rm.rm_tag = csv_data.get_value(i, k++);
        rm.source_path = csv_data.get_value(i, k++);
        rm.top_module = csv_data.get_value(i, k++);
        
        rm_list.push_back(rm);
        k = 0;
    }
}

void pr_tool::prep_proj_directory()
{
    int status, i;
/*
    //TODO: check if directory exists
    status = mkdir(project_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(source_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/project").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/constraints").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/cores").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((source_path + "/netlist").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Synth").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Implement").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Checkpoint").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir((project_dir + "/Bitstreams").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(hdl_copy_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    status = mkdir(tcl_project.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
*/

    //TODO: check if directory exists
    fs::create_directories(Project_dir);
    fs::create_directories(Src_path);
    fs::create_directories(Src_path + "/project");
    fs::create_directories(Src_path + "/constraints");
    fs::create_directories(Src_path + "/cores");
    fs::create_directories(Src_path + "/netlist");
    fs::create_directories(Project_dir + "/Synth");
    fs::create_directories(Project_dir + "/Implement");
    fs::create_directories(Project_dir + "/Checkpoint");
    fs::create_directories(Project_dir + "/Bitstreams");
    fs::create_directories(hdl_copy_path);
    fs::create_directories(tcl_project);

    //TODO: 1. assert if directory/files exists
    //      2. remove leading or trailing space from source_path

    //copy source files of each RM to Source/core/top_name
    for(i = 0; i < num_rm_modules; i++){
        std::string str = Src_path + "/cores/";
        fs::create_directories(str);
        fs::copy(rm_list[i].source_path, str, fs::copy_options::recursive); 
    } 
    
    fs::copy("tools/Tcl", tcl_project, fs::copy_options::recursive); 
    fs::copy("tools/load_prj.py", Src_path, fs::copy_options::recursive); 
    fs::copy("tools/start_vivado", Project_dir, fs::copy_options::recursive); 

    //create the .prj file using the python script
}


void pr_tool::generate_synthesis_tcl()
{
     unsigned long i;
     ofstream write_synth_tcl;
     synthesis_script = Project_dir + "/ooc_synth.tcl";
     //tcl_home = folder + "/TCL";

     write_synth_tcl.open(synthesis_script); 

     write_synth_tcl << "set tclParams [list hd.visual 1]" <<endl;
     write_synth_tcl << "set tclHome " << "\"" << tcl_project << "\"" <<endl;
     write_synth_tcl << "set tclDir $tclHome" <<endl;
     write_synth_tcl << "set projDir " <<"\"" << Project_dir << "\"" <<endl;
     write_synth_tcl <<" source $tclDir/design_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/log_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/synth_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/impl_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/pr_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/log_utils.tcl" <<endl;
     write_synth_tcl <<" source $tclDir/hd_floorplan_utils.tcl" <<endl;

     write_synth_tcl << "############################################################### \n" <<
                        "### Define Part, Package, Speedgrade \n" <<
                        "###############################################################"<<endl;

    //TODO:automate FPGA type here
     write_synth_tcl << "set part xc7z010clg400-1" <<endl;
     write_synth_tcl << "check_part $part" <<endl;

     write_synth_tcl << "####flow control" <<endl;
     write_synth_tcl << "set run.topSynth       1" <<endl;
     write_synth_tcl << "set run.rmSynth        1" <<endl;
     write_synth_tcl << "set run.prImpl         0" <<endl;
     write_synth_tcl << "set run.prVerify       0" <<endl;
     write_synth_tcl << "set run.writeBitstream 0" <<endl;

     write_synth_tcl << "####Report and DCP controls - values: 0-required min; 1-few extra; 2-all" <<endl;
     write_synth_tcl << "set verbose      1" <<endl;
     write_synth_tcl << "set dcpLevel     1" <<endl;

     write_synth_tcl<<" ####Output Directories" <<endl;
     write_synth_tcl<<" set synthDir  $projDir/Synth" <<endl;
     write_synth_tcl<<" set implDir   $projDir/Implement" <<endl;
     write_synth_tcl<<" set dcpDir    $projDir/Checkpoint" <<endl;
     write_synth_tcl<<" set bitDir    $projDir/Bitstreams" <<endl;

     write_synth_tcl<<" ####Input Directories "<<endl;
     write_synth_tcl<<" set srcDir     $projDir/Sources" <<endl;
     write_synth_tcl<<" set rtlDir     $srcDir/hdl" <<endl;
     write_synth_tcl<<" set prjDir     $srcDir/project" <<endl;
     write_synth_tcl<<" set xdcDir     $srcDir/xdc" <<endl;
     write_synth_tcl<<" set coreDir    $srcDir/cores" <<endl;
     write_synth_tcl<<" set netlistDir $srcDir/netlist" <<endl;

     write_synth_tcl<<"####################################################################" <<endl;
     write_synth_tcl<<"### RP Module Definitions" <<endl;
     write_synth_tcl<<" ####################################################################" <<endl;

     for(i = 0; i < num_rm_modules; i++) {
         write_synth_tcl << "add_module " << rm_list[i].rm_tag <<endl;
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " moduleName\t" << rm_list[i].top_module <<endl;
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " prj \t" << "$prjDir/" << rm_list[i].rm_tag <<".prj" <<endl;
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " synth \t" << "${run.rmSynth}" <<endl;
         write_synth_tcl <<endl;
     }

     write_synth_tcl << "source $tclDir/run.tcl" <<endl;
     write_synth_tcl.close(); 
}

void pr_tool::start_synthesis(std::string synth_script)
{   
    chdir(("cd " + Project_dir).c_str());
    cout << "Current path is " << fs::current_path() <<endl;
    fs::current_path(Src_path);
    cout << "Changed path is " << fs::current_path() <<endl;
    std::system(("python3.6  load_prj.py"));
    fs::current_path(Project_dir);
    std::string vivado_path = "start_vivado";
    std::system(("./"+ vivado_path + " " + synth_script).c_str());
}

void pr_tool::parse_synthesis_report()
{
    unsigned long i, k = 0;
    string lut = "Slice LUTs*";
    string dsp = "DSPs";
    string bram = "Block RAM Tile    |";
    vector<slot> extracted_res = vector<slot>(num_rm_modules);
   
    cout << "Parsing Synth utilization report "  <<endl;
    for(i = 0; i < num_rm_modules; i++) {
        string line, word;
        ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                      rm_list[i].top_module + "_utilization_synth.rpt");

        //cout <<"filename is" << Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
        //              rm_list[i].top_module + "_utilization_synth.rpt" <<endl;

        while (getline(file, line)) {
            if(line.find(lut) != string::npos) {
                stringstream iss(line);

                while(iss >> word) {
                    k++;
                    if(k == 5){
                        extracted_res[i].clb= (unsigned long)((std::stoul(word) / 8));
                        cout << " clb " <<  extracted_res[i].clb <<endl;
                    }
                }
                k = 0;
            }

            if(line.find(bram) != string::npos) {
                stringstream iss(line);

                while(iss >> word) {
                    k++;

                    if(k == 6) {
                        extracted_res[i].bram = std::stoul(word); 
                        cout <<" bram " <<  extracted_res[i].bram <<endl;
                    }
                }
                k = 0;
            }

            if(line.find(dsp) != string::npos){
                stringstream iss(line);

                while(iss >> word) {
                    k++;

                    if(k == 4){
                        extracted_res[i].dsp = std::stoul(word);
                        cout << " dsp " <<  extracted_res[i].dsp <<endl;
                    }
                }
                k = 0;
            }
        }
    }
        ofstream write_flora_input;
        write_flora_input.open(Project_dir +"/flora_input.csv");
        
        for(i = 0; i < num_rm_modules; i++){
            write_flora_input <<extracted_res[i].clb <<","  << extracted_res[i].bram 
                              << "," <<extracted_res[i].dsp <<"," <<
#ifdef WITH_PARTITIONING
                                HW_WCET[i] << "," <<slacks[i] <<"," <<
#endif
                                rm_list[i].rm_tag <<endl;
        }
        
        write_flora_input.close();        
        input_to_flora in_flora = {num_rm_modules, type, Project_dir +"/flora_input.csv"};

        fl_inst = new flora(&in_flora);
        fl_inst->clear_vectors();
        fl_inst->prep_input();
        fl_inst->start_optimizer();
        fl_inst->generate_xdc();
}
