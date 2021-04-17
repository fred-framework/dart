#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "pr_tool.h"
#include <fstream>

#undef RE_WRAP

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
        
        //pre-process the design
        prep_input(); 
    	init_dir_struct();

        prep_proj_directory();

#ifdef WITHOUT_PARTITIONING
        generate_wrapper(fl_inst);
#endif
        //generate synthesis script 
        create_vivado_project();       
        generate_synthesis_tcl(fl_inst);

        //syntehsize reconfigurable accelerators
        run_vivado(synthesis_script);

        //extract resource consumption of accelerators
        parse_synthesis_report();
 
        //perform floorplanning/partitioning
        fl_inst = new flora(&in_flora);
        fl_inst->clear_vectors();       
        fl_inst->prep_input();
        fl_inst->start_optimizer();
        fl_inst->generate_xdc(fplan_xdc_file);

        //generate static hardware
        generate_static_part(fl_inst);

        //synthesize static part
        synthesize_static();

#ifdef WITH_PARTITIONING
        //re_synthesis_after_wrap = 1;
        generate_wrapper(fl_inst);
        create_vivado_project();       
        generate_synthesis_tcl(fl_inst);
        run_vivado(synthesis_script);
#endif

        //generate implementation script
        generate_impl_tcl(fl_inst);

        //run vivado implementation
        run_vivado(impl_script);    
      
        //generate the run-time management files for FRED
        generate_fred_files(fl_inst);
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

void pr_tool::generate_fred_files(flora *fl_ptr)
{
    int k, i, bitstream_id = 100;
    unsigned long partitions;
    std::string str, src, dest;
    unsigned long fred_input_buff_size = 1048576;
    unsigned long fred_output_buff_size = 32768;

    ofstream write_fred_arch, write_fred_hw;
    write_fred_arch.open(fred_dir +"/arch.csv");
    write_fred_hw.open(fred_dir +"/hw_tasks.csv");
   
    cout << "PR_TOOL: creating FRED files "<<endl;

    /*  FRED architectural description file header */
    write_fred_arch <<"# FRED Architectural description file. \n";
    write_fred_arch <<"# Warning: This file must match synthesized hardware! \n \n";
    write_fred_arch <<"# Each line defines a partition, syntax: \n";
    write_fred_arch <<"# <partition name>, <num slots> \n \n";
    write_fred_arch <<"# example: \n";
    write_fred_arch <<"# \"ex_partition, 3\" \n";
    write_fred_arch <<"# defines a partion named \"ex_partition\" containing 3 slots\n";

    /*  HW_tasks description file header*/
    write_fred_hw << "# FRED hw-tasks description file. \n ";
    write_fred_hw << "# Warning: This file must match synthesized hardware! \n \n";
    write_fred_hw << "# Each line defines a HW-Tasks: \n ";
    write_fred_hw << "# <name>, <hw_id>, <partition>, <bistream_path>, <buff_0_size>, ... <buff_7_size> \n ";
    write_fred_hw << "# Note: the association between a hw-task and its partition \n ";
    write_fred_hw << "# it's defined during the synthesis flow! Here is specified only \n ";
    write_fred_hw << "# to guess the number of bistreams and their length. \n \n ";
    write_fred_hw << "# example: \n ";
    write_fred_hw << "# \"ex_hw_task, 64, ex_partition, bits, 1024, 1024, 1024\" \n ";
    write_fred_hw << "# defines a hw-task named \"ex_hw_task\", with id 64, allocated on a \n ";
    write_fred_hw << "# partition named \"ex_partition\", whose bitstreams are located in \n ";
    write_fred_hw << "# the \"/bits\" folder, and uses three input/output buffers of size 1024 bytes. \n \n ";

#ifdef WITH_PARTITIONING
        for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
            write_fred_arch << "p"<<k << ", "  << fl_ptr->alloc[k].num_hw_tasks_in_part << "\n";
        }

        for(i = 0; i < fl_ptr->from_solver.max_modules_per_partition; i++) {
            for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
                write_fred_hw << "config_"<<i<<"_pblock_slot_"<<k<<"_partial, " <<bitstream_id << ", p"<<k<<", dart_fred/bits, " <<fred_input_buff_size <<", " << fred_output_buff_size <<"\n";
                bitstream_id++;
            }
        }

    /* Create the FRED bitstream partition directories */
    for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
        str = "fred/dart_fred/bits/p"+ std::to_string(k);
        fs::create_directories(str);
    }

    /* Copy the partial bitstreams */
    for(i = 0; i < fl_ptr->from_solver.max_modules_per_partition; i++) {
        for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
            src = "Bitstreams/config_" + std::to_string(i) + "_pblock_slot_" + std::to_string(k) + "_partial.bin";
            dest = "fred/dart_fred/bits/p"+ std::to_string(k);
            fs::copy(src, dest);
        }
    }

    /*Copy one of the static bitstreams to FRED*/
    fs::copy("Bitstreams/config_0.bin", "fred/dart_fred/bits/static.bin");

    write_fred_arch.close();
    write_fred_hw.close();

#else
    /* Create the arch.csv and hw_tasks.csv files for FRED */
    for(k = 0; k < num_rm_partitions; k++) {
        write_fred_arch << "p"<<k << ", "  << alloc[k].num_hw_tasks_in_part << "\n";
    }

    for(i = 0; i < max_modules_in_partition; i++) {
        for(k = 0; k < num_rm_partitions; k++) {
            write_fred_hw << "config_"<<i<<"_pblock_slot_"<<k<<"_partial, " <<bitstream_id << ", p"<<k<<", dart_fred/bits, " <<fred_input_buff_size <<", " << fred_output_buff_size <<"\n";
            bitstream_id++;
        }
    }
   
    /* Create the FRED bitstream partition directories */
    for(k = 0; k < num_rm_partitions; k++) {
        str = "fred/dart_fred/bits/p"+ std::to_string(k);
        fs::create_directories(str);
    }

    /* Copy the partial bitstreams */
    for(i = 0; i < max_modules_in_partition; i++) {
        for(k = 0; k < num_rm_partitions; k++) {
            src = "Bitstreams/config_" + std::to_string(i) + "_pblock_slot_" + std::to_string(k) + "_partial.bin"; 
            dest = "fred/dart_fred/bits/p"+ std::to_string(k);
            fs::copy(src, dest);
        }
    }

    /*Copy one of the static bitstreams to FRED*/
    fs::copy("Bitstreams/config_0.bin", "fred/dart_fred/bits/static.bin");

    write_fred_arch.close();
    write_fred_hw.close();
#endif
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
        //rm.source_path = csv_data.get_value(i, k++);
        rm.top_module = csv_data.get_value(i, k++);
        
        rm_list.push_back(rm);
        k = 0;
    }

#ifndef WITH_PARTITIONING 
    for(k = 0; k < num_rm_partitions; k++) {
        for(i = 0; i < num_rm_modules; i++) {
            if(rm_list[i].partition_id  == k){
                alloc[k].num_modules_in_partition += 1;
                alloc[k].num_hw_tasks_in_part += 1;
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
        fs::create_directories(Src_path / fs::path("ip_repo"));
        fs::create_directories(Project_dir / fs::path("Synth"));
        fs::create_directories(Project_dir / fs::path("Implement"));
        fs::create_directories(Project_dir / fs::path("Checkpoint"));
        fs::create_directories(Project_dir / fs::path("Bitstreams"));
        fs::create_directories(ip_repo_path);
        fs::create_directories(hdl_copy_path);
        fs::create_directories(tcl_project);
        fs::create_directories(fred_dir);
        fs::create_directories(fred_dir / fs::path("dart_fred/bits"));
        fs::create_directories(static_dir);

        // getting the path to the IPs. this has been checked in the main
        char * val = getenv( "DART_IP_PATH" );
        string dart_ip_path  = val == NULL ? std::string("") : std::string(val);

        //TODO: 1. assert if directory/files exists
        //      2. remove leading or trailing space from source_path

        //copy source files of each RM to Source/core/top_name
        for(i = 0; i < num_rm_modules; i++){
            std::string str = Src_path / fs::path("cores") / rm_list[i].rm_tag;
            fs::create_directories(str);
            fs::copy(fs::path(dart_ip_path) / rm_list[i].rm_tag, str, fs::copy_options::recursive); 
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
        dir_source = dart_path / fs::path("tools") / fs::path("acc_bbox_ip");
        fs::copy(dir_source, ip_repo_path, fs::copy_options::recursive);
/*
        fs::copy(dir_source, dir_dest, fs::copy_options::recursive);
        // copy the DCP provided by the user
        // the dcp file must be renamed to <original_name>_synth.dcp
        string dcp_filename = fs::path(input_pr->static_dcp_file).filename().replace_extension("");
        // the following lines seems complex, but it's only dir_dest + orig_filename + _synth + orig_extension
        dcp_filename = string(dir_dest) + string (fs::path("/")) + dcp_filename + "_synth" + string(fs::path(input_pr->static_dcp_file).extension());
        fs::copy_file(input_pr->static_dcp_file, dcp_filename, fs::copy_options::overwrite_existing);
*/
    }catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    } 
}

void pr_tool::generate_synthesis_tcl(flora *fl_ptr)
{
     unsigned long i, j, k;
     ofstream write_synth_tcl;
     fs::path script(Project_dir);
     script /= fs::path("ooc_synth.tcl");
     synthesis_script = script.string();
     std::string tag, top_mod;
     //tcl_home = folder + "/TCL";

     cout << "PR_TOOL: creating synthesis scripts "<<endl;
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

#ifdef WITH_PARTITIONING
    if (re_synthesis_after_wrap == 1) {
        for(i = 0; i < fl_ptr->from_solver.num_partition; i++) {
            for(k = 0; k <  fl_ptr->alloc[i].num_hw_tasks_in_part; k++) {          
                tag = rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag;
                write_synth_tcl << "add_module " << tag <<endl;
                write_synth_tcl << "set_attribute module " <<tag << " moduleName\t" << wrapper_top_name << "_" << i <<endl;
                write_synth_tcl << "set_attribute module " <<tag << " prj \t" << "$prjDir/" << tag <<".prj" <<endl;
                write_synth_tcl << "set_attribute module " <<tag << " synth \t" << "${run.rmSynth}" <<endl;
                write_synth_tcl <<endl;
            }
        }
    }
    
    else {     
         for(i = 0; i < num_rm_modules; i++) {
             write_synth_tcl << "add_module " << rm_list[i].rm_tag <<endl;
             write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " moduleName\t" << rm_list[i].top_module <<endl;
             write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " prj \t" << "$prjDir/" << rm_list[i].rm_tag <<".prj" <<endl;
             write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " synth \t" << "${run.rmSynth}" <<endl;
             write_synth_tcl <<endl;
         }
     }
#else
     for(i = 0; i < num_rm_modules; i++) {
         write_synth_tcl << "add_module " << rm_list[i].rm_tag <<endl;
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " moduleName\t" << wrapper_top_name << "_" << rm_list[i].partition_id  <<endl;
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " prj \t" << "$prjDir/" << rm_list[i].rm_tag <<".prj" <<endl;                   
         write_synth_tcl << "set_attribute module " <<rm_list[i].rm_tag << " synth \t" << "${run.rmSynth}" <<endl;
         write_synth_tcl <<endl;
     }
#endif
     write_synth_tcl << "source $tclDir/run.tcl" <<endl;
     write_synth_tcl << "exit" <<endl;
     write_synth_tcl.close(); 
     
     re_synthesis_after_wrap = 1;
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

#ifdef WITH_PARTITIONING 
            if(re_synthesis_after_wrap == 1) {
                prj_file << "system xil_defaultlib ./Sources/cores/"<<ip_name<<"/hdl/verilog/wrapper_top.v" <<endl;
                vhd_file_cnt++;
            }
#else
            prj_file << "system xil_defaultlib ./Sources/cores/"<<ip_name<<"/hdl/verilog/wrapper_top.v" <<endl;
            vhd_file_cnt++;
#endif

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
                              wrapper_top_name + "_" + to_string(rm_list[i].partition_id) + "_utilization_synth.rpt" <<endl;

                ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                              wrapper_top_name + "_" + to_string(rm_list[i].partition_id) + "_utilization_synth.rpt");

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
        in_flora = {num_rm_modules, Project_dir +"/flora_input.csv", input_pr->static_top_module};
#else
        //in_flora = {num_rm_partitions, Project_dir +"/flora_input.csv", input_pr->static_top_modul};
        in_flora = {num_rm_partitions, Project_dir +"/flora_input.csv"};
#endif
}

void pr_tool::generate_impl_tcl(flora *fl_ptr)
{ 
    unsigned long i, k, partition_ptr, temp_index = 0;
    ofstream write_impl_tcl;
    string config_name, tag;
    impl_script = Project_dir + "/impl.tcl";
    string implement = "implement", import = "import";
     
    cout << "PR_TOOL: creating implementation script "<<endl;

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

     write_impl_tcl<< "set top \"dart_wrapper\"" <<endl; 
/*
     write_impl_tcl<< "set top \"" << input_pr->static_top_module << "_wrapper\"" <<endl; 
*/
     write_impl_tcl<< "set static \"Static\" "<<endl;
     write_impl_tcl<< "add_module $static" <<endl;
     write_impl_tcl<< "set_attribute module $static moduleName    $top" <<endl;
     write_impl_tcl<< "set_attribute module $static top_level     1" <<endl;
     write_impl_tcl<< "set_attribute module $static synth         ${run.topSynth}" <<endl;

     write_impl_tcl<<"####################################################################" <<endl;
     write_impl_tcl<<"### RP Module Definitions" <<endl;
     write_impl_tcl<<" ####################################################################" <<endl;

#ifdef WITH_PARTITIONING
/*    for(i = 0; i < num_rm_modules; i++) {
        write_impl_tcl << "add_module " << rm_list[i].rm_tag <<endl;
        write_impl_tcl << "set_attribute module " <<rm_list[i].rm_tag << " moduleName\t" << rm_list[i].top_module <<endl;
        write_impl_tcl <<endl;
     }
    */
//    if (re_synthesis_after_wrap == 1) {
        for(i = 0; i < fl_ptr->from_solver.num_partition; i++) {
            for(k = 0; k <  fl_ptr->alloc[i].num_hw_tasks_in_part; k++) {
                tag = rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag;
                write_impl_tcl << "add_module " << tag <<endl;
                write_impl_tcl << "set_attribute module " <<tag << " moduleName\t" << wrapper_top_name << "_" << i <<endl;
                
           }
        }
//    }
#else
    for(i = 0; i < num_rm_modules; i++) {
        write_impl_tcl << "add_module " << rm_list[i].rm_tag <<endl;
        write_impl_tcl << "set_attribute module "  << rm_list[i].rm_tag <<  " moduleName\t" << wrapper_top_name << "_" << rm_list[i].partition_id <<endl;
        write_impl_tcl <<endl;
     }
#endif

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
    ip_repo_path =Src_path + "/ip_repo";
    hdl_copy_path = Src_path + "/hdl";
    fplan_xdc_file = Src_path + "/constraints/pblocks.xdc";
    tcl_project = Project_dir + "/Tcl";
    synthesis_script = Project_dir + "/ooc_synth.tcl" ;
    impl_script = Project_dir + "/impl.tcl";
    fred_dir = Project_dir + "/fred";
    static_dir = Project_dir + "/static_hw";
}

void pr_tool::generate_static_part(flora *fl_ptr) 
{
    int i, j, k;
    unsigned int num_partitions = 0;
    ofstream write_static_tcl;

#ifdef WITH_PARTITIONING
    num_partitions = fl_ptr->from_solver.num_partition;
#else
    num_partitions = num_rm_partitions;
#endif
    
    static_hw_script = Project_dir + "/static.tcl";
    write_static_tcl.open(static_hw_script);

    //create the project
#ifdef FPGA_PYNQ
    write_static_tcl << "create_project dart_project -force " << static_dir << " -part xc7z020clg400-1 " <<endl;  
#elif FPGA_ZYNQ
    write_static_tcl << "create_project dart_project -force " << static_dir << " -part xc7z010clg400-1 " <<endl;
#else
    write_static_tcl << "create_project dart_project -force " << static_dir << " -part xc7z010clg400-1 " <<endl;
#endif

    write_static_tcl << "set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project] " <<endl; //TODO: define Board
    write_static_tcl << "set_property  ip_repo_paths "<<ip_repo_path<<" [current_project]" <<endl;
    write_static_tcl << "update_ip_catalog " <<endl;
    write_static_tcl << "create_bd_design \"dart\" " <<endl;
    write_static_tcl << "update_compile_order -fileset sources_1 " <<endl;
    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0" <<endl; //TODO: PS must be templated
    write_static_tcl << "endgroup " <<endl;
    
    //create the bbox instance of the accelerator IPs and the decouplers (two decouplers for each acc)
    for(i=0, j=0; i < num_partitions; i++, j++) {
        write_static_tcl << "startgroup " <<endl;
        write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:hls:acc:1.0 acc_" <<std::to_string(i) <<endl;
        write_static_tcl << "endgroup " <<endl;
        write_static_tcl << "startgroup " <<endl;
        write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:pr_decoupler:1.0 pr_decoupler_"<<std::to_string(j) <<endl; //for Vivado 2019.2 and below
        //write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 pr_decoupler_"<<std::to_string(j) <<endl;
        write_static_tcl << "endgroup " <<endl;
        write_static_tcl << "set_property -dict [list CONFIG.ALL_PARAMS {HAS_AXI_LITE 1 HAS_SIGNAL_CONTROL 0 INTF {acc_ctrl "
                            "{ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 PROTOCOL axi4lite SIGNALS {ARVALID {PRESENT 1} "
                            "ARREADY {PRESENT 1} AWVALID {PRESENT 1} AWREADY {PRESENT 1} BVALID {PRESENT 1} BREADY {PRESENT 1} "
                            "RVALID {PRESENT 1} RREADY {PRESENT 1} WVALID {PRESENT 1} WREADY {PRESENT 1} AWADDR {PRESENT 1} "
                            "AWLEN {PRESENT 0} AWSIZE {PRESENT 0} AWBURST {PRESENT 0} AWLOCK {PRESENT 0} AWCACHE {PRESENT 0} "
                            "AWPROT {PRESENT 1} WDATA {PRESENT 1} WSTRB {PRESENT 1} WLAST {PRESENT 0} BRESP {PRESENT 1} "
                            "ARADDR {PRESENT 1} ARLEN {PRESENT 0} ARSIZE {PRESENT 0} ARBURST {PRESENT 0} ARLOCK {PRESENT 0} "
                            "ARCACHE {PRESENT 0} ARPROT {PRESENT 1} RDATA {PRESENT 1} RRESP {PRESENT 1} RLAST {PRESENT 0}}}}} "
                            "CONFIG.GUI_HAS_AXI_LITE {1} CONFIG.GUI_HAS_SIGNAL_CONTROL {0} CONFIG.GUI_SELECT_INTERFACE {0} "
                            "CONFIG.GUI_INTERFACE_NAME {acc_ctrl} CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} "
                            "CONFIG.GUI_INTERFACE_PROTOCOL {axi4lite} CONFIG.GUI_SIGNAL_SELECT_0 {ARVALID} CONFIG.GUI_SIGNAL_SELECT_1 {ARREADY} "
                            "CONFIG.GUI_SIGNAL_SELECT_2 {AWVALID} CONFIG.GUI_SIGNAL_SELECT_3 {AWREADY} CONFIG.GUI_SIGNAL_SELECT_4 {BVALID} "
                            "CONFIG.GUI_SIGNAL_SELECT_5 {BREADY} CONFIG.GUI_SIGNAL_SELECT_6 {RVALID} CONFIG.GUI_SIGNAL_SELECT_7 {RREADY} "
                            "CONFIG.GUI_SIGNAL_SELECT_8 {WVALID} CONFIG.GUI_SIGNAL_SELECT_9 {WREADY} CONFIG.GUI_SIGNAL_DECOUPLED_0 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_1 {true} CONFIG.GUI_SIGNAL_DECOUPLED_2 {true} CONFIG.GUI_SIGNAL_DECOUPLED_3 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_4 {true} CONFIG.GUI_SIGNAL_DECOUPLED_5 {true} CONFIG.GUI_SIGNAL_DECOUPLED_6 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_7 {true} CONFIG.GUI_SIGNAL_DECOUPLED_8 {true} CONFIG.GUI_SIGNAL_DECOUPLED_9 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_0 {true} CONFIG.GUI_SIGNAL_PRESENT_1 {true} CONFIG.GUI_SIGNAL_PRESENT_2 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_3 {true} CONFIG.GUI_SIGNAL_PRESENT_4 {true} CONFIG.GUI_SIGNAL_PRESENT_5 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_6 {true} CONFIG.GUI_SIGNAL_PRESENT_7 {true} CONFIG.GUI_SIGNAL_PRESENT_8 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_9 {true}] [get_bd_cells pr_decoupler_" << std::to_string(j)<< "]" <<endl;
        j++; 
        write_static_tcl << "startgroup" << endl;
        write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:pr_decoupler:1.0 pr_decoupler_"<<std::to_string(j) << endl; //for Vivado 2019.2 and below
        //write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 pr_decoupler_"<<std::to_string(j) << endl;
        write_static_tcl << "endgroup" << endl;
    
        write_static_tcl << " set_property -dict [list CONFIG.ALL_PARAMS {HAS_SIGNAL_CONTROL 1 HAS_SIGNAL_STATUS 0 INTF "
                            "{acc_data {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave}}} CONFIG.GUI_HAS_SIGNAL_CONTROL {1} " 
                            "CONFIG.GUI_HAS_SIGNAL_STATUS {0} CONFIG.GUI_SELECT_INTERFACE {0} CONFIG.GUI_INTERFACE_NAME {acc_data} "
                            "CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} CONFIG.GUI_INTERFACE_PROTOCOL {axi4} " 
                            "CONFIG.GUI_SELECT_MODE {slave} CONFIG.GUI_SIGNAL_SELECT_0 {ARVALID} CONFIG.GUI_SIGNAL_SELECT_1 {ARREADY} "
                            "CONFIG.GUI_SIGNAL_SELECT_2 {AWVALID} CONFIG.GUI_SIGNAL_SELECT_3 {AWREADY} CONFIG.GUI_SIGNAL_SELECT_4 {BVALID} "
                            "CONFIG.GUI_SIGNAL_SELECT_5 {BREADY} CONFIG.GUI_SIGNAL_SELECT_6 {RVALID} CONFIG.GUI_SIGNAL_SELECT_7 {RREADY} "
                            "CONFIG.GUI_SIGNAL_SELECT_8 {WVALID} CONFIG.GUI_SIGNAL_SELECT_9 {WREADY} CONFIG.GUI_SIGNAL_DECOUPLED_0 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_1 {true} CONFIG.GUI_SIGNAL_DECOUPLED_2 {true} CONFIG.GUI_SIGNAL_DECOUPLED_3 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_4 {true} CONFIG.GUI_SIGNAL_DECOUPLED_5 {true} CONFIG.GUI_SIGNAL_DECOUPLED_6 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_7 {true} CONFIG.GUI_SIGNAL_DECOUPLED_8 {true} CONFIG.GUI_SIGNAL_DECOUPLED_9 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_0 {true} CONFIG.GUI_SIGNAL_PRESENT_1 {true} CONFIG.GUI_SIGNAL_PRESENT_2 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_3 {true} CONFIG.GUI_SIGNAL_PRESENT_4 {true} CONFIG.GUI_SIGNAL_PRESENT_5 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_6 {true} CONFIG.GUI_SIGNAL_PRESENT_7 {true} CONFIG.GUI_SIGNAL_PRESENT_8 {true} "
                            "CONFIG.GUI_SIGNAL_PRESENT_9 {true}] [get_bd_cells pr_decoupler_"<< std::to_string(j) << "]" <<endl;
    }
  
    //connect PS to DDR and Fixed IO
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external \"FIXED_IO, DDR\" "
                        "apply_board_preset \"1\" Master \"Disable\" Slave \"Disable\" }  [get_bd_cells processing_system7_0]" <<endl;

    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0] " <<endl; //TODO: number of HPO should be changed based on number of acc
    write_static_tcl << "endgroup " <<endl;

    //create AXI bars
    //For now we create two axi bars: one for the control registers and the other for memory access 
    write_static_tcl << "startgroup" <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0" <<endl;
    write_static_tcl << "endgroup" <<endl;
    write_static_tcl << "startgroup" <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_1" <<endl;
    write_static_tcl << "endgroup" <<endl;

    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.NUM_MI {"<< std::to_string(2*num_partitions) << "}] [get_bd_cells axi_interconnect_0]" <<endl;
    write_static_tcl << "endgroup" <<endl;
    write_static_tcl << "startgroup" <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.NUM_SI {"<<std::to_string(num_partitions)<< "} CONFIG.NUM_MI {1}] [get_bd_cells axi_interconnect_1]" <<endl;
    write_static_tcl << "endgroup" <<endl;

    
    for(i=0, j=0; i < num_partitions; i++, j++) {
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins acc_"<<std::to_string(i)<<"/s_axi_ctrl_bus] [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_ctrl]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/rp_acc_ctrl] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M0"<<std::to_string(j)<<"_AXI]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M0"<<std::to_string(j+1)<<"_AXI]" <<endl;
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]" <<endl; 
        
        j++;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_data] [get_bd_intf_pins acc_"<<std::to_string(i)<<"/m_axi_mem_bus]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/rp_acc_data] -boundary_type upper [get_bd_intf_pins axi_interconnect_1/S0"<<std::to_string(i)<<"_AXI]" <<endl;
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_1/M00_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]" <<endl;
    
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j-1)<<"/decouple_status] [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/decouple]" <<endl;
        
        //connect acc clk
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} "
                            "[get_bd_pins acc_"<<std::to_string(i)<<"/ap_clk]" <<endl;
    }
    
    //connect interconnects to master clock
    write_static_tcl << "startgroup" <<endl;
    for(i=0, j=0; i < num_partitions; i++, j+=2) {
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j+1)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/S0"<<std::to_string(i)<<"_ACLK]" <<endl;
    }

    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/ACLK]" <<endl;
    //write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M00_ACLK]" <<endl;
    //write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M01_ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/S00_ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/M00_ACLK]" <<endl;
    //write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/S00_ACLK]" <<endl;
    write_static_tcl << "endgroup" <<endl;

    for(j=0; j < num_partitions * 2; j+=2) { 
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<
        "/s_axi_reg_aresetn] [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] " <<endl;
    }
    
    //Address map for accelerators
    //TODO: Generate the linux device tree from this mapping
    for(i=0, j=0; i < num_partitions; i++, j+=2) {
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {acc_"<<std::to_string(i)<<"/s_axi_ctrl_bus/Reg }]" <<endl;
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg/Reg }]" <<endl;
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {processing_system7_0/S_AXI_HP0/HP0_DDR_LOWOCM }]" <<endl;
    }

    //connect interrupts
    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 " <<endl;
    write_static_tcl << "endgroup " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.NUM_PORTS {"<<num_partitions<<"}] [get_bd_cells xlconcat_0]" <<endl;
    write_static_tcl << "startgroup" <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.PCW_USE_FABRIC_INTERRUPT {1} CONFIG.PCW_IRQ_F2P_INTR {1}] [get_bd_cells processing_system7_0]" <<endl;
    write_static_tcl << "endgroup" <<endl;
    write_static_tcl << "connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/IRQ_F2P]" <<endl;

    for(i=0, j=0; i < num_partitions; i++, j+=2) {
        write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<i<<"/interrupt] [get_bd_pins xlconcat_0/In"<<i<<"] " <<endl;
    }

    //create a wrapper
    write_static_tcl << "make_wrapper -files [get_files "<< static_dir <<"/dart_project.srcs/sources_1/bd/dart/dart.bd] -top " <<endl;
    write_static_tcl << "add_files -norecurse "<< static_dir<<"/dart_project.srcs/sources_1/bd/dart/hdl/dart_wrapper.v " <<endl;

    write_static_tcl << "save_bd_design  " <<endl;

    write_static_tcl << "update_compile_order -fileset sources_1" <<endl;
    write_static_tcl << "set_property synth_checkpoint_mode None [get_files  "<<static_dir<<"/dart_project.srcs/sources_1/bd/dart/dart.bd]" << endl;
    write_static_tcl << "generate_target all [get_files  "<< static_dir <<"/dart.srcs/sources_1/bd/dart/dart.bd]" <<endl;

    //create synthesis runs in vivado
    write_static_tcl << "reset_run synth_1" <<endl;
    write_static_tcl << "launch_runs synth_1 -jobs 8" <<endl;
    write_static_tcl << "wait_on_run -timeout 360 synth_1" <<endl;
}

void pr_tool::synthesize_static()
{
    std::string src = static_dir +"/dart_project.runs/synth_1/dart_wrapper.dcp";
    std::string dest = Project_dir + "/Synth/Static/dart_wrapper_synth.dcp";
    run_vivado(static_hw_script);
    
    try {
        fs::copy(src, dest);
    }catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    }
}

void pr_tool::generate_wrapper(flora *fl_ptr)
{
    int i,j,k;

    cout << "PR_TOOL: generating HDL wrappers " <<endl;

#ifndef WITH_PARTITIONING

    for(i = 0; i < num_rm_modules; i++) {
        create_acc_wrapper(rm_list[i].top_module, rm_list[i].rm_tag, rm_list[i].partition_id );
    }
#else
    for(i = 0; i < fl_ptr->from_solver.num_partition; i++) {
        for(k = 0; k <  fl_ptr->alloc[i].num_hw_tasks_in_part; k++) {                                                      
            create_acc_wrapper(rm_list[fl_ptr->alloc[i].task_id[k]].top_module, 
                                rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag, 
                                i);
        }
    }
/*
    if(fl_ptr->alloc[partition_ptr].num_tasks_in_part > 0) {
        fl_ptr->alloc[partition_ptr].num_tasks_in_part--;                                                                                          
        write_impl_tcl <<"\t \t \t \t \t";                                                                                                         
        write_impl_tcl <<"[list " << rm_list[fl_ptr->alloc[partition_ptr].task_id[temp_index]].rm_tag                                              
        <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;                                                                 
    }
                
*/
#endif
}
