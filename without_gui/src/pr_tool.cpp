#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "pr_tool.h"
#include <fstream>

using namespace std;

pr_tool::pr_tool(input_to_pr *pr_input) 
{
    input_pr = pr_input; 
    dart_path = getenv("DART_HOME");
    cout << "PR_TOOL: Starting PR_tool " <<endl;

#ifdef WITH_PARTITIONING
    if(input_pr->num_rm_modules > 0){
        num_rm_modules = pr_input->num_rm_modules;
#else
    if(input_pr->num_rm_partitions > 0){
        num_rm_partitions = pr_input->num_rm_partitions;
#endif

#ifdef FPGA_ZYNQ
    type = TYPE_ZYNQ;
#elif FPGA_PYNQ 
    type = TYPE_PYNQ;
#else
    type = TYPE_ZYNQ;
#endif

#ifdef WITH_PARTITIONING
        cout << "PR_TOOL: num of slots  **** " << num_rm_modules <<endl;
#else
        cout << "PR_TOOL: num of partitions **** " << num_rm_partitions <<endl;
#endif        
        cout << "PR_TOOL: type of FPGA pr_tool **** " << type <<endl;
        cout << "PR_TOOL: path for input pr_tool **** " << pr_input->path_to_input <<endl;
        cout << "PR_TOOL: path for output pr_tool **** " << pr_input->path_to_output <<endl;
        
        //Instantiate flora
        //flora fl_inst;

        prep_input(); 
    	init_dir_struct();

        prep_proj_directory();
        generate_synthesis_tcl();
        create_vivado_project();       

        run_vivado(synthesis_script);

        parse_synthesis_report();
 
        fl_inst = new flora(&in_flora);
        fl_inst->clear_vectors();       
        fl_inst->prep_input();
        fl_inst->start_optimizer();
        fl_inst->generate_xdc(fplan_xdc_file);

        generate_impl_tcl(fl_inst);

        run_vivado(impl_script);    
  }
    else {
        cout <<"PR_TOOL: The number of Reconfigurable modules > 0";
        exit(-1);
    }    
}

pr_tool::~pr_tool()
{
    cout << "PR_TOOL: destruction of PR_tool" <<endl;
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

    cout << endl << "PR_TOOL: reading inputs " << row << " " << col <<endl;
    num_rm_modules = row;
    for(i = 0, ptr = 0, k = 0; i < num_rm_modules; i++, ptr++) {

#ifdef WITH_PARTITIONING
        str = csv_data.get_value(i, k++);
        HW_WCET.push_back(std::stod(str));
        str = csv_data.get_value(i, k++);
        slacks.push_back(std::stod(str));
#else
        str = csv_data.get_value(i, k++);
        rm.partition_id = std::stoul(str);
#endif
        rm.rm_tag = csv_data.get_value(i, k++);
        rm.source_path = csv_data.get_value(i, k++);
        rm.top_module = csv_data.get_value(i, k++);
        
        rm_list.push_back(rm);
        k = 0;
    }

#ifndef WITH_PARTITIONING 
    for(k = 0; k < num_rm_partitions; k++) {
        for(i = 0; i < num_rm_modules; i++) {
            if(rm_list[i].partition_id  == k){
                alloc[k].num_modules_in_partition += 1;
                alloc[k].rm_id.push_back(i);
            }
        }
        if(max_modules_in_partition < alloc[k].num_modules_in_partition)
            max_modules_in_partition = alloc[k].num_modules_in_partition;
    }

#endif
}


void pr_tool::prep_proj_directory()
{
    int status, i;
    
    try{
        cout << "PR_TOOL: creating project directory "<<endl;
        //TODO: check if directory exists
        //fs::create_directories(Project_dir);
        fs::create_directories(Src_path);
        fs::create_directories(Src_path / fs::path("project"));
        fs::create_directories(Src_path / fs::path("constraints"));
        fs::create_directories(Src_path / fs::path("cores"));
        fs::create_directories(Src_path / fs::path("netlist"));
        fs::create_directories(Project_dir / fs::path("Synth"));
        fs::create_directories(Project_dir / fs::path("Implement"));
        fs::create_directories(Project_dir / fs::path("Checkpoint"));
        fs::create_directories(Project_dir / fs::path("Bitstreams"));
        fs::create_directories(hdl_copy_path);
        fs::create_directories(tcl_project);

        //TODO: 1. assert if directory/files exists
        //      2. remove leading or trailing space from source_path

        //copy source files of each RM to Source/core/top_name
        for(i = 0; i < num_rm_modules; i++){
            std::string str = Src_path / fs::path("cores") / rm_list[i].rm_tag;
            fs::create_directories(str);
            fs::copy(fs::path(rm_list[i].source_path) / rm_list[i].rm_tag, str, fs::copy_options::recursive); 
        } 
        fs::path dir_source(dart_path);
        dir_source /= fs::path("tools") / fs::path("Tcl");
        fs::copy(dir_source, tcl_project, fs::copy_options::recursive); 
        //fs::copy("tools/load_prj.py", Src_path, fs::copy_options::recursive); 
        dir_source = dart_path / fs::path("tools") / fs::path("start_vivado");
        fs::copy(dir_source, Project_dir, fs::copy_options::recursive); 
        dir_source = dart_path / fs::path("tools") / fs::path("synth_static");
        fs::path dir_dest(Project_dir);
        dir_dest /= fs::path("Synth") / fs::path("Static");
        fs::copy(dir_source, dir_dest, fs::copy_options::recursive);
    }catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    } 
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
#ifdef FPGA_PYNQ
     write_synth_tcl << "set part xc7z020clg400-1" <<endl;
#elif FPGA_ZYNQ
     write_synth_tcl << "set part xc7z010clg400-1" <<endl;
#else
     write_synth_tcl << "set part xc7z010clg400-1" <<endl;
#endif

    write_synth_tcl << "check_part $part" <<endl;

     write_synth_tcl << "####flow control" <<endl;
     write_synth_tcl << "set run.topSynth       0" <<endl;
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
     write_synth_tcl << "exit" <<endl;
     write_synth_tcl.close(); 
}

void pr_tool::create_vivado_project()
{ 
    try{
        chdir(("cd " + Project_dir).c_str());
        fs::current_path(Src_path);
        // using the fs::path in order to have an, OS independent, safe path string
        // taking into account the dir separator
        fs::path  dir = fs::path("cores");
        if (!fs::exists(dir) || !fs::is_directory(dir)){
            cout << "ERROR: '" << dir.string() << "' directory not found.\n";
            exit(1);
        }
        
        dir = fs::path("project");
        if (!fs::exists(dir) || !fs::is_directory(dir)){
            cout << "ERROR: '" << dir.string() << "' directory not found.\n";
            exit(1);
        }

        // iterates over every dirs in 'cores'
        for (const auto & entry : fs::directory_iterator("cores")){
            if (!fs::is_directory(entry.path())){
                cout << "ERROR: expecting only directories with IPs inside the 'cores' dir.\n";
                exit(1);
            }

            // entry.path() returns 'cores/myip', but only the last part is required
            // so, i need to split the string
            string ip_name = entry.path();
            vector<string> aux = split(ip_name,fs::path::preferred_separator);
            ip_name = aux[aux.size()-1];

            // open prj file for writing 
            ofstream prj_file;
            fs::path prj_file_name("project");
            prj_file_name /= (ip_name + ".prj");
            prj_file.open(prj_file_name.string());

            // check the IP dir
            fs::path vhd_path("cores");
            vhd_path /= ip_name / fs::path("hdl") / fs::path("vhdl");
            if (!fs::exists(vhd_path) || !fs::is_directory(vhd_path)){
                cout << "ERROR: '" << vhd_path.string() << "' directory not found.\n";
                exit(1);
            }

            // write all vhd file names into the prj file
            int vhd_file_cnt=0;
            for (const auto & entry2 : fs::directory_iterator(vhd_path)){
                if (fs::is_regular_file(entry2) && (fs::path(entry2).extension() == ".vhd")){
                    prj_file << "vhdl xil_defaultlib ./Sources/" + entry2.path().string() +"\n";
                    vhd_file_cnt++;
                }
            }
            cout << "PR_TOOL: IP '" << ip_name << "' has " << vhd_file_cnt << " files\n";
        }
    }
    catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    }      
}

void pr_tool::run_vivado(std::string synth_script)
{ 
    chdir(("cd " + Project_dir).c_str());
    //fs::current_path(Src_path);
    //std::system(("python3.6  load_prj.py"));
    // the python file execution has been replaced by create_vivado_project()
    fs::current_path(Project_dir);
    fs::path bash_script(Project_dir);
    bash_script /= fs::path("start_vivado");
    if (!fs::is_regular_file(bash_script)){
        cout << "ERROR: " << bash_script.string() << " not found\n";
        exit(1);
    }
    fs::path vivado_script(synth_script);
    if (!fs::is_regular_file(vivado_script)){
        cout << "ERROR: " << vivado_script.string() << " not found\n";
        exit(1);
    }
    // run vivado
    try{
        std::system((bash_script.string() + " " + vivado_script.string()).c_str());
    }
    catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    }   
}


void pr_tool::parse_synthesis_report()
{
    unsigned long i, k, j = 0;
    string lut = "Slice LUTs*";
    string dsp = "DSPs";
    string bram = "Block RAM Tile    |";
    vector<slot> extracted_res = vector<slot>(num_rm_modules);
   
    cout << "PR_TOOL: Parsing Synth utilization report "  <<endl;
    
#ifdef WITH_PARTITIONING
    for(i = 0; i < num_rm_modules; i++) {
        string line, word;
        cout <<"PR_TOOL:filename is" << Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                      rm_list[i].top_module + "_utilization_synth.rpt" <<endl;

        ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                      rm_list[i].top_module + "_utilization_synth.rpt");

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

#else
    unsigned long max_clb, max_bram, max_dsp;    
    for(j = 0; j < num_rm_partitions; j++) {
        max_clb = 0;
        max_bram = 0;
        max_dsp = 0;
        for(i = 0; i < num_rm_modules; i++) {
            string line, word;

            if(rm_list[i].partition_id == j) {
                k = 0;

                cout <<"PR_TOOL:filename is " << Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                              rm_list[i].top_module + "_utilization_synth.rpt" <<endl;

                ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                              rm_list[i].top_module + "_utilization_synth.rpt");

                while (getline(file, line)) {
                    if(line.find(lut) != string::npos) {
                        stringstream iss(line);
                        
                        while(iss >> word) {
                            k++;
                            if(k == 5){
                                if(max_clb < (std::stoul(word) / 8))
                                    max_clb = ((std::stoul(word) / 8));

                                 cout << " LUTs " <<  word <<endl; 
                            }
                        }
                        k = 0;
                    }

                    if(line.find(bram) != string::npos) {
                        stringstream iss(line);

                        while(iss >> word) {
                            k++;

                            if(k == 6) {
                                if(stod(word) > 0.0 && max_bram <  (std::stoul(word) + 1))
                                    //if(max_bram <  (std::stoul(word) + 1))
                                        max_bram = std::stoul(word) + 1;
                                 else if (max_bram <  (std::stoul(word)))
                                    max_bram = std::stoul(word);
                                
                                cout <<" BRAM " <<  word <<endl;
                            }
                        }
                        k = 0;
                    }

                    if(line.find(dsp) != string::npos){
                        stringstream iss(line);

                        while(iss >> word) {
                            k++;

                            if(k == 4){
                                if(max_dsp < std::stoul(word))
                                    max_dsp = std::stoul(word);
                                    
                                cout << " DSP " <<  word <<endl;
                            }
                        }
                        k = 0;
                    }
                }
            }
        }
        
        extracted_res[j].clb = max_clb;
        extracted_res[j].bram = max_bram;
        extracted_res[j].dsp = max_dsp;
    
    }

#endif    
    ofstream write_flora_input;
        write_flora_input.open(Project_dir +"/flora_input.csv");
#ifdef WITH_PARTITIONING         
        for(i = 0; i < num_rm_modules; i++){
#else
        for(i = 0; i < num_rm_partitions; i++){
#endif
            write_flora_input <<extracted_res[i].clb + CLB_MARGIN <<"," ; 
        
            if(extracted_res[i].bram > 0)
                write_flora_input << extracted_res[i].bram + BRAM_MARGIN <<",";
            else
                write_flora_input << extracted_res[i].bram << "," ;

            if(extracted_res[i].dsp > 0)
                write_flora_input << extracted_res[i].dsp + DSP_MARGIN <<"," ;
            else
                write_flora_input << extracted_res[i].dsp <<"," ;

#ifdef WITH_PARTITIONING
            write_flora_input << HW_WCET[i] << "," <<slacks[i] <<"," ;
#endif
            write_flora_input << rm_list[i].rm_tag <<endl;
        }
        
        write_flora_input.close();        
#ifdef WITH_PARTITIONING 
        in_flora = {num_rm_modules, Project_dir +"/flora_input.csv"};
#else
        in_flora = {num_rm_partitions, Project_dir +"/flora_input.csv"};
#endif
}

void pr_tool::generate_impl_tcl(flora *fl_ptr)
{ 
    unsigned long i, k, partition_ptr, temp_index = 0;
    ofstream write_impl_tcl;
    string config_name;
    impl_script = Project_dir + "/impl.tcl";
    string implement = "implement", import = "import";

    write_impl_tcl.open(impl_script);

     write_impl_tcl << "set tclParams [list hd.visual 1]" <<endl;
     write_impl_tcl << "set tclHome " << "\"" << tcl_project << "\"" <<endl;
     write_impl_tcl << "set tclDir $tclHome" <<endl;
     write_impl_tcl << "set projDir " <<"\"" << Project_dir << "\"" <<endl;
     write_impl_tcl <<" source $tclDir/design_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/log_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/synth_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/impl_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/pr_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/log_utils.tcl" <<endl;
     write_impl_tcl <<" source $tclDir/hd_floorplan_utils.tcl" <<endl;

     write_impl_tcl << "############################################################### \n" <<
                        "### Define Part, Package, Speedgrade \n" <<
                        "###############################################################"<<endl;

    //TODO:automate FPGA type here
#ifdef FPGA_PYNQ
     write_impl_tcl << "set part xc7z020clg400-1" <<endl;
#elif FPGA_ZYNQ
     write_impl_tcl << "set part xc7z010clg400-1" <<endl;
#else
     write_impl_tcl << "set part xc7z010clg400-1" <<endl;
#endif

    write_impl_tcl << "check_part $part" <<endl;

     write_impl_tcl << "####flow control" <<endl;
     write_impl_tcl << "set run.topSynth       0" <<endl;
     write_impl_tcl << "set run.rmSynth        0" <<endl;
     write_impl_tcl << "set run.prImpl         1" <<endl;
     write_impl_tcl << "set run.prVerify       1" <<endl;
     write_impl_tcl << "set run.writeBitstream 1" <<endl;

     write_impl_tcl << "####Report and DCP controls - values: 0-required min; 1-few extra; 2-all" <<endl;
     write_impl_tcl << "set verbose      1" <<endl;
     write_impl_tcl << "set dcpLevel     1" <<endl;

     write_impl_tcl<<" ####Output Directories" <<endl;
     write_impl_tcl<<" set synthDir  $projDir/Synth" <<endl;
     write_impl_tcl<<" set implDir   $projDir/Implement" <<endl;
     write_impl_tcl<<" set dcpDir    $projDir/Checkpoint" <<endl;
     write_impl_tcl<<" set bitDir    $projDir/Bitstreams" <<endl;

     write_impl_tcl<<" ####Input Directories "<<endl;
     write_impl_tcl<<" set srcDir     $projDir/Sources" <<endl;
     write_impl_tcl<<" set rtlDir     $srcDir/hdl" <<endl;
     write_impl_tcl<<" set prjDir     $srcDir/project" <<endl;
     write_impl_tcl<<" set xdcDir     $srcDir/xdc" <<endl;
     write_impl_tcl<<" set coreDir    $srcDir/cores" <<endl;
     write_impl_tcl<<" set netlistDir $srcDir/netlist" <<endl;

     write_impl_tcl<<"####################################################################" <<endl;
     write_impl_tcl<<"### Top Module Definitions" <<endl;
     write_impl_tcl<<" ####################################################################" <<endl;

//     write_impl_tcl<< "set top \"design_1_wrapper\"" <<endl; 
//     write_impl_tcl<< "set top \"hdmi_out_wrapper\"" <<endl; 
//     write_impl_tcl<< "set top \"system_wrapper_2_slots\"" <<endl; 
     write_impl_tcl<< "set top \"system_wrapper_1_slots\"" <<endl; 
     write_impl_tcl<< "set static \"Static\" "<<endl;
     write_impl_tcl<< "add_module $static" <<endl;
     write_impl_tcl<< "set_attribute module $static moduleName    $top" <<endl;
     write_impl_tcl<< "set_attribute module $static top_level     1" <<endl;
     write_impl_tcl<< "set_attribute module $static synth         ${run.topSynth}" <<endl;

     write_impl_tcl<<"####################################################################" <<endl;
     write_impl_tcl<<"### RP Module Definitions" <<endl;
     write_impl_tcl<<" ####################################################################" <<endl;


    for(i = 0; i < num_rm_modules; i++) {
        write_impl_tcl << "add_module " << rm_list[i].rm_tag <<endl;
        write_impl_tcl << "set_attribute module " <<rm_list[i].rm_tag << " moduleName\t" << rm_list[i].top_module <<endl;
        write_impl_tcl <<endl;
     }

#ifdef WITH_PARTITIONING
    for(i = 0, k = 0; i < fl_ptr->from_solver.max_modules_per_partition; i++, k++) {
        write_impl_tcl << "############################################################### \n" <<
                           "###Implemenetation configuration " << i <<endl <<
                           "###############################################################"<<endl;

        config_name = "config_" + to_string(i);
        write_impl_tcl <<"add_implementation "<<config_name <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" top \t   $top" <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" pr.impl \t 1" <<endl;

        write_impl_tcl <<"set_attribute impl "<<config_name <<" implXDC \t [list " << fplan_xdc_file << "]" <<endl;

        /*TODO: add the implementation of the static part */
        
        write_impl_tcl <<"set_attribute impl "<<config_name << " partitions \t";
        if(i == 0 )
            write_impl_tcl <<"[list [list $static           $top \t" + implement + "   ] \\" <<endl;
        else
            write_impl_tcl <<"[list [list $static           $top \t" + import   + "   ] \\" <<endl;
        for(partition_ptr = 0; partition_ptr <  fl_ptr->from_solver.num_partition; partition_ptr++) {
            if(fl_ptr->alloc[partition_ptr].num_tasks_in_part > 0) {
                fl_ptr->alloc[partition_ptr].num_tasks_in_part--;
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << rm_list[fl_ptr->alloc[partition_ptr].task_id[temp_index]].rm_tag 
                   <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
            }

            else {
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << rm_list[fl_ptr->alloc[partition_ptr].task_id[0]].rm_tag
                               <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" import] \\" <<endl;
            }
        }
        write_impl_tcl <<"]"<<endl;
        write_impl_tcl <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" impl \t    ${run.prImpl} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" verify \t   ${run.prVerify} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" bitstream \t ${run.writeBitstream} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" bitstream_options    \"-bin_file\"" <<endl;
        temp_index += 1;
    }

#else    
    for(i = 0, k = 0; i < max_modules_in_partition; i++, k++) {
        write_impl_tcl << "############################################################### \n" <<
                           "###Implemenetation configuration " << i <<endl <<
                           "###############################################################"<<endl;

        config_name = "config_" + to_string(i);
        write_impl_tcl <<"add_implementation "<<config_name <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" top \t   $top" <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" pr.impl \t 1" <<endl;

        write_impl_tcl <<"set_attribute impl "<<config_name <<" implXDC \t [list " << fplan_xdc_file << "]" <<endl;

        /*TODO: add the implementation of the static part */
        
        write_impl_tcl <<"set_attribute impl "<<config_name << " partitions \t";
        if(i == 0 )
            write_impl_tcl <<"[list [list $static           $top \t" + implement + "   ] \\" <<endl;
        else
            write_impl_tcl <<"[list [list $static           $top \t" + import   + "   ] \\" <<endl;
        for(partition_ptr = 0; partition_ptr <  num_rm_partitions; partition_ptr++) {
            if(alloc[partition_ptr].num_modules_in_partition > 0) {
                alloc[partition_ptr].num_modules_in_partition--;
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << rm_list[alloc[partition_ptr].rm_id[temp_index]].rm_tag 
                   <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
            }
            else {
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << rm_list[alloc[partition_ptr].rm_id[0]].rm_tag 
                               <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" import] \\" <<endl;

            
            }
        }
        write_impl_tcl <<"]"<<endl;
        write_impl_tcl <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" impl \t    ${run.prImpl} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" verify \t   ${run.prVerify} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" bitstream \t ${run.writeBitstream} " <<endl;
        write_impl_tcl <<"set_attribute impl "<<config_name <<" bitstream_options    \"-bin_file\"" <<endl;
        temp_index += 1;
    }
#endif

    write_impl_tcl << "source $tclDir/run.tcl" <<endl;
    write_impl_tcl << "exit"<<endl;
    write_impl_tcl.close();
}

void pr_tool::init_dir_struct()
{
    Project_dir = input_pr->path_to_output;

    cout << "PR_TOOL: Project directory is " << Project_dir <<endl;

    //creating project sub-directories
    //Project_dir += "/pr_tool_proj"; 
    Src_path = Project_dir + "/Sources";
    hdl_copy_path = Src_path + "/hdl";
    fplan_xdc_file = Src_path + "/constraints/pblocks.xdc";
    tcl_project = Project_dir + "/Tcl";
    synthesis_script = Project_dir + "/ooc_synth.tcl" ;
    impl_script = Project_dir + "/impl.tcl";;

}

