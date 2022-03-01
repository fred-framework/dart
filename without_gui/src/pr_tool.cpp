#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <utility> // std::as_const
#include "pr_tool.h"
#include "version.h"
#include <yaml-cpp/yaml.h>

#undef RE_WRAP

using namespace std;

extern YAML::Node config;

pr_tool::pr_tool(string path_to_output) 
//pr_tool::pr_tool(input_to_pr *pr_input) 
{
    int i;
    //input_pr = pr_input; 
    dart_path = getenv("DART_HOME");
    std::cout << "PR_TOOL: Starting PR_tool " <<endl;
    Project_dir = path_to_output;

#ifdef WITH_PARTITIONING
    //if(input_pr->num_rm_modules > 0){
    //    num_rm_modules = pr_input->num_rm_modules;
    // count the number of IPs in all partitions
    num_rm_modules = config["dart"]["hw_ips"].size();    
    cout << endl << "PR_TOOL: reading inputs " << num_rm_modules << endl;
#else
    //if(input_pr->num_rm_partitions > 0){
        //num_rm_partitions = pr_input->num_rm_partitions;
    num_rm_partitions = config["dart"]["partitions"].size();
    if (num_rm_partitions <= 0) {
        cout <<"PR_TOOL: The number of Reconfigurable modules > 0";
        exit(EXIT_FAILURE);
    } 
    num_rm_modules = 0;
    for(i = 0; i < num_rm_partitions; i++) {
        num_rm_modules += config["dart"]["partitions"][i]["hw_ips"].size();
    }         
#endif

#ifdef FPGA_ZYNQ
    type = TYPE_ZYNQ;
#elif FPGA_PYNQ 
    type = TYPE_PYNQ;
#elif FPGA_ZCU_102
    type = TYPE_US;
#elif FPGA_US_96
    type = TYPE_US_96;
#else
    type = TYPE_ZYNQ;
#endif

#ifdef WITH_PARTITIONING
        cout << "PR_TOOL: num of slots  **** " << num_rm_modules <<endl;
#else
        cout << "PR_TOOL: num of partitions **** " << num_rm_partitions <<endl;
#endif        
        cout << "PR_TOOL: type of FPGA pr_tool **** " << type <<endl;
        //cout << "PR_TOOL: type of FPGA pr_tool **** " << fpga_type_name[type] <<endl;
        //cout << "PR_TOOL: path for input pr_tool **** " << pr_input->path_to_input <<endl;
        cout << "PR_TOOL: path for output pr_tool **** " <<     Project_dir  <<endl;
        
        //Instantiate flora
        //flora fl_inst;

        //pre-process the design
        //prep_input(); 
    	init_dir_struct();

        prep_proj_directory();

#ifdef WITHOUT_PARTITIONING
        generate_wrapper(fl_inst);
#endif
        //generate synthesis script 
        create_vivado_project();       
        generate_synthesis_tcl(fl_inst);

        //syntehsize reconfigurable accelerators
        //TODO (amory): this part could be executed in parallel for each IP
        if (!config["skip_ip_synthesis"] || !config["skip_ip_synthesis"].as<bool>()){
            run_vivado(synthesis_script);
        }
 
        //extract resource consumption of accelerators
        parse_synthesis_report();

        //perform floorplanning/partitioning
        //fl_inst = new flora(&in_flora);
        fl_inst = new flora();
        fl_inst->clear_vectors();       
        fl_inst->prep_input();
        fl_inst->start_optimizer();

        // generate the instance name, used in the xdc file and in the impl.tcl
        #ifdef WITH_PARTITIONING
            fl_inst->generate_cell_name(from_solver.num_partition);
        #else
            fl_inst->generate_cell_name(num_rm_partitions);
        #endif

        // the user might want to ignore DART floorplanning/partitioning optimization and use his own pblock XDc file
        if (config["pblocks_xdc"]){
            fs::path xdc_file( config["pblocks_xdc"].as<string>() );
            // the file existance has been tested before
            fs::copy_file(xdc_file.string(), fplan_xdc_file,fs::copy_options::update_existing);
        } else {
            fl_inst->generate_xdc(fplan_xdc_file);
        }

        //generate static hardware
        generate_static_part(fl_inst);

        //synthesize static part
        if (!config["skip_static_synthesis"] || !config["skip_static_synthesis"].as<bool>()){
            synthesize_static();
        }

#ifdef WITH_PARTITIONING
        //re_synthesis_after_wrap = 1;
        generate_wrapper(fl_inst);
        create_vivado_project();       
        generate_synthesis_tcl(fl_inst);
        run_vivado(synthesis_script);
#else
        // TODO: currently, ILA are inserted only in the partitioning OFF mode
        // If one partition (or an IP, in partitioning mode) uses ILA, then debug probes must be generated 
        for (std::size_t i=0;i<num_rm_partitions;i++) {
            if(config["dart"]["partitions"][i]["debug"]){
                add_debug_probes();
                break;
            }
        }           
#endif

        //generate implementation script
        generate_impl_tcl(fl_inst);

        //run vivado implementation
        if (!config["skip_implementation"] || !config["skip_implementation"].as<bool>()){
            run_vivado(impl_script);
        }

        //generate the run-time management files for FRED
        generate_fred_files(fl_inst);
        generate_fred_device_tree(fl_inst);  
}

pr_tool::~pr_tool()
{
    cout << "PR_TOOL: destruction of PR_tool" <<endl;
}

void pr_tool::add_debug_probes()
{
    ofstream write_debug_tcl;
    string debug_script = Project_dir + "/Tcl/debug_probes.tcl";
     
    cout << "PR_TOOL: creating debug probe script "<<endl;

    write_debug_tcl.open(debug_script);

    write_debug_tcl << "open_checkpoint " << Project_dir << "/Synth/Static/dart_wrapper_synth.dcp"<<endl;
    write_debug_tcl << "write_debug_probes -force " << static_dir << "/debug_probes.ltx"<<endl;
    write_debug_tcl << "exit "<<endl;
    write_debug_tcl.close();

    run_vivado(debug_script);   
}

void pr_tool::generate_fred_device_tree(flora *fl_ptr)
{
    int i, j, k;
    unsigned int num_partitions = 0;
    // the 1st interrupt signal has value 89, the 2nd 90, and so on
    unsigned int first_interrupt = 0x1d;
    unsigned int first_reg_addr = 0xc0;
    ofstream write_dev_tree;

#ifdef WITH_PARTITIONING
    num_partitions = fl_ptr->from_solver.num_partition;
#else
    num_partitions = num_rm_partitions;
#endif
    
    write_dev_tree.open(fred_dir + "/static.dts");
    
    write_dev_tree << "/* DART generated device tree overlay */"<<endl;
#if defined (FPGA_ZCU_102) || defined (FPGA_US_96)
    write_dev_tree <<"/dts-v1/;" <<endl;
    write_dev_tree <<"/plugin/;" <<endl;
    write_dev_tree <<"/ { \n" <<endl;
    write_dev_tree << "/* FRED static support design */ " <<endl;
	write_dev_tree <<"\tfragment@0 { "<<endl;
	write_dev_tree <<"\t\ttarget = <&fpga_full>;"<<endl;
	write_dev_tree <<"\t\toverlay0: __overlay__ {"<<endl;
	write_dev_tree <<"\t\t\t#address-cells = <2>;" <<endl;
	write_dev_tree <<"\t\t\t#size-cells = <2>;" <<endl;
	write_dev_tree <<"\t\t\tfirmware-name = \"static.bin\";" <<endl;
	write_dev_tree <<"\t\t\tresets = <&zynqmp_reset 116>;"<<endl;
    write_dev_tree <<"\t\t};" <<endl;
	write_dev_tree <<"\t};" <<endl;
    write_dev_tree <<endl;
	
    // write_dev_tree <<"\t/* Disable PYNQ base */" <<endl;
	// write_dev_tree <<"\tfragment@1 {"<<endl;
	// write_dev_tree <<"\t\ttarget-path = \"/amba/fabric@A0000000\";" <<endl;
	// write_dev_tree <<"\t\t\toverlay1: __overlay__ {" <<endl;
	// write_dev_tree <<"\t\t\t\tstatus = \"disabled\";" <<endl;
	// write_dev_tree <<"\t\t\t};" <<endl;
	// write_dev_tree <<"\t\t};" <<endl;
    // write_dev_tree <<endl;
	
    
    // write_dev_tree <<"\t/* PL base configuration */ " <<endl;
	// write_dev_tree <<"\tfragment@2 {" <<endl;
	// write_dev_tree <<"\t\ttarget = <&amba>;" <<endl;
	// write_dev_tree <<"\t\t\toverlay2: __overlay__ {" <<endl;
	// write_dev_tree <<"\t\t\t\tafi0: afi0 {" <<endl;
	// write_dev_tree <<"\t\t\t\t\tcompatible = \"xlnx,afi-fpga\";" <<endl;
	// write_dev_tree <<"\t\t\t\t\tconfig-afi = < 0 0>, <1 0>, <2 0>, <3 0>, <4 0>, <5 0>, <6 0>, <7 0>, <8 0>, <9 0>, <10 0>, <11 0>, <12 0>, <13 0>, <14 0xa00>, <15 0x000>;" <<endl;
	// write_dev_tree <<"\t\t\t\t};" <<endl;
	// write_dev_tree <<"\t\t\t\tclocking0: clocking0 {" <<endl;
	// write_dev_tree <<"\t\t\t\t\t#clock-cells = <0>;" <<endl;
	// write_dev_tree <<"\t\t\t\t\tassigned-clock-rates = <100000000>;" <<endl;
	// write_dev_tree <<"\t\t\t\t\tassigned-clocks = <&zynqmp_clk 71>;" <<endl;
	// write_dev_tree <<"\t\t\t\t\tclock-output-names = \"fabric_clk\"; " <<endl;
	// write_dev_tree <<"\t\t\t\t\tclocks = <&zynqmp_clk 71>;" <<endl;
	// write_dev_tree <<"\t\t\t\t\tcompatible = \"xlnx,fclk\"; "<< endl;
	// write_dev_tree <<"\t\t\t\t};" <<endl;
	// write_dev_tree <<"\t\t\t};" <<endl;
	// write_dev_tree <<"\t\t};" <<endl;
    
    
    write_dev_tree <<"/* FRED slots layout */"<<endl;
    write_dev_tree <<"fragment@3 {"<<endl;
    write_dev_tree <<"\ttarget = <&amba>;" <<endl;
    write_dev_tree <<"\t\toverlay3: __overlay__ {"<<endl;

    //write_dev_tree <<"\tamba { \n" <<endl;
    write_dev_tree <<"\t\t\t#address-cells = <2>;" <<endl;
    write_dev_tree <<"\t\t\t#size-cells = <2>;" <<endl;

    for (i = 0; i < num_partitions * 2; i+=2) {
        // write_dev_tree <<"\t\t\tslot_p"<<i<<"_s0@A00"<< std::hex << i <<"0000 { " <<endl; 
        // write_dev_tree <<"\t\t\t\tcompatible = \"generic-uio\";" <<endl;
        // write_dev_tree <<"\t\t\t\treg = <0xA00"<< std::hex << i<<"0000 0x10000>;" <<endl;
        // write_dev_tree <<"\t\t\t\tinterrupt-parent = <0x4>;" <<endl;
        // write_dev_tree <<"\t\t\t\tinterrupts = <0x0 0x"<< std::hex << first_interrupt + i <<" 0x4>; " <<endl;
        // write_dev_tree <<"\t\t\t\t}; " <<endl;
        // write_dev_tree <<endl;

        write_dev_tree <<"\t\t\tslot_p"<<i<<"_s0@A00"<< std::hex << i <<"0000 { " <<endl;
		//write_dev_tree <<"\t\t\t\tclock-names = \"ap_clk\";" <<endl;
		//write_dev_tree <<"\t\t\t\tclocks = <&zynqmp_clk 71>;" <<endl;
		write_dev_tree <<"\t\t\t\tcompatible = \"generic-uio\";" <<endl;
		write_dev_tree <<"\t\t\t\treg = <0x0 0xA00"<< std::hex << i<<"0000 0x0 0x10000>;" <<endl;
		//write_dev_tree <<"\t\t\t\txlnx,s-axi-ctrl-bus-addr-width = <0x8>;" <<endl;
		//write_dev_tree <<"\t\t\t\txlnx,s-axi-ctrl-bus-data-width = <0x20>;" <<endl;
		write_dev_tree <<"\t\t\t\tinterrupt-parent = <&gic>;" <<endl;
		write_dev_tree <<"\t\t\t\tinterrupts = <0 " << first_interrupt + i <<" 4>;" <<endl;
        write_dev_tree <<"\t\t\t}; " <<endl;
        write_dev_tree <<endl;

        // write_dev_tree <<"\t\t\tpr_decoupler_p"<<i<<"_s0@A00"<<std::hex << i+1<<"0000 { " <<endl;
        // write_dev_tree <<"\t\t\t\tcompatible = \"generic-uio\";" <<endl;
        // write_dev_tree <<"\t\t\t\treg = <0xA00"<< std::hex << i+1 <<"0000 0x10000>;" <<endl;
        // write_dev_tree <<"\t\t\t\t}; " <<endl;
        // write_dev_tree <<endl;
        // write_dev_tree <<"\t\t}; " <<endl;

        write_dev_tree <<"\t\t\tpr_decoupler_p"<<i<<"_s0@A00"<<std::hex << i+1<<"0000 { " <<endl;
		//write_dev_tree <<"\t\t\t\tclock-names = \"aclk\";" <<endl;
		//write_dev_tree <<"\t\t\t\tclocks = <&zynqmp_clk 71>;" <<endl;
		write_dev_tree <<"\t\t\t\tcompatible = \"generic-uio\";" <<endl;
		write_dev_tree <<"\t\t\t\treg = <0x0 0xA00"<<std::hex << i+1<<"0000 0x0 0x10000>;" <<endl;         
        write_dev_tree <<"\t\t\t}; " <<endl;
    }
    write_dev_tree <<"\t\t}; " <<endl;
    write_dev_tree <<"\t}; " <<endl;
    write_dev_tree <<"}; " <<endl;

#endif

#if defined(FPGA_PYNQ) || defined(FPGA_ZYNQ)
    write_dev_tree <<"/ {"<<endl;
    write_dev_tree <<"\tamba {"<<endl;
    write_dev_tree << endl;
    for (i = 0; i < num_partitions * 2; i+=2) {
        write_dev_tree <<"\t\tslot_p"<<i<<"_s0@43"<<std::hex << first_reg_addr + i <<"0000 { " <<endl;
        write_dev_tree <<"\t\t\tcompatible = \"generic-uio\";" <<endl;
        write_dev_tree <<"\t\t\treg = <0x43" <<std::hex << first_reg_addr + i <<"0000 0x10000>;" <<endl; 
        write_dev_tree <<"\t\t\tinterrupt-parent = <0x4>;" <<endl;
        write_dev_tree <<"\t\t\tinterrupts = <0x0 0x"<< std::hex << first_interrupt + i <<" 0x4>; " <<endl;
        write_dev_tree <<"\t\t}; " <<endl;
        write_dev_tree <<endl;

        write_dev_tree <<"\t\tpr_decoupler_p"<<i<<"_s0@43"<<std::hex << first_reg_addr + i + 1<<"0000 { " <<endl;
        write_dev_tree <<"\t\t\tcompatible = \"generic-uio\";" <<endl;
        write_dev_tree <<"\t\t\treg =  <0x43" <<std::hex << first_reg_addr + i + 1 <<"0000 0x10000>;" <<endl; 
        write_dev_tree <<"\t\t}; " <<endl;
        write_dev_tree <<endl; 
    }   
    write_dev_tree <<"\t}; " <<endl;
    write_dev_tree <<"}; " <<endl;
#endif
    
}

void pr_tool::generate_fred_files(flora *fl_ptr)
{
    int k, i,j, bitstream_id = 100;
    unsigned long partitions;
    std::string str, src, dest;
    fs::current_path(Project_dir);
    try {
    
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
        write_fred_hw << "# Fred-Linux hw-tasks description file. \n ";
        write_fred_hw << "# Warning: This file must match synthesized hardware! \n ";
        write_fred_hw << "# \n ";
        write_fred_hw << "# Each line defines a HW-Tasks with the following syntax: \n ";
        write_fred_hw << "# <name>, <id>, <timeout_ms>, <partition>, <bits_path>, <buff_0_size>, ... <buff_7_size> \n ";
        write_fred_hw << "# \n ";
        write_fred_hw << "# For each hw-task, the field: \n ";
        write_fred_hw << "# - <name> is the name of the hw-task and must match the name of the bitstream files; \n ";
        write_fred_hw << "# - <id> is the hw-id of the hw-task; \n ";
        write_fred_hw << "# - <timeout_ms> defines an execution timeout for the hw-task in milliseconds; \n ";
        write_fred_hw << "# - <partition> is the name of the partition to which it belongs; \n ";
        write_fred_hw << "# - <bits_path> define the relative path of the bitstreams directory for the hw-task; \n ";
        write_fred_hw << "# - <buff_0_size>, ... <buff_7_size> define the number and sizes fo the data buffers. \n ";
        write_fred_hw << "# \n ";
        write_fred_hw << "# Example: \n ";
        write_fred_hw << "# \"ex-hw-task, 64, ex-partition, bitstreams, 100, 1024, 1024, 1024\" \n ";
        write_fred_hw << "# defines a hw-task named \"ex-hw-task\" \n ";
        write_fred_hw << "# - having id 64, \n ";
        write_fred_hw << "# - belonging to a partition named \"ex-partition\", \n ";
        write_fred_hw << "# - whose bitstreams are located in the \"./bitstreams\" directory, \n ";
        write_fred_hw << "# - having an execution timeout of 100 ms, \n ";
        write_fred_hw << "# - using three input/output buffers of size 1024 bytes each. \n ";


        int n_buffers=0;
        #ifdef WITH_PARTITIONING
            for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
                //write_fred_arch << "p"<<k << ", "  << fl_ptr->alloc[k].num_hw_tasks_in_part << "\n";
                write_fred_arch << "p"<<k << ", "  << "1\n";
            }

            int ip_id;
            for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
                for(i = 0; i < fl_ptr->from_solver.max_modules_per_partition; i++) {
                    if (i < fl_ptr->alloc[k].num_hw_tasks_in_part) {
                        //write_fred_hw << rm_list[fl_ptr->alloc[k].task_id[i]].rm_tag <<", " <<bitstream_id << ", p"<<k<<", dart_fred/bits, " <<fred_input_buff_size <<", " << fred_output_buff_size <<"\n";
                        ip_id = fl_ptr->alloc[k].task_id[i];
                        write_fred_hw << config["dart"]["hw_ips"][ip_id]["ip_name"].as<string>() <<", " <<bitstream_id <<
                            ", " << config["dart"]["hw_ips"][ip_id]["timeout"].as<uint>() << ", p"<<k<<", dart_fred/bits, ";
                        bitstream_id++;
                        n_buffers = config["dart"]["hw_ips"][ip_id]["buffers"].size();
                        for(j = 0; j < n_buffers; j++) {
                            write_fred_hw << config["dart"]["hw_ips"][ip_id]["buffers"][j].as<int>();
                            if (j < (n_buffers-1)){
                                write_fred_hw << ", ";
                            }
                        }
                        write_fred_hw << "\n";
                    }
                }
            }

            /* Create the FRED bitstream partition directories */
            for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
                str = "fred/dart_fred/bits/p"+ std::to_string(k);
                fs::create_directories(str);
            }

            /* Copy the partial bitstreams */
            for(k = 0; k < fl_ptr->from_solver.num_partition; k++) {
                for(i = 0; i < fl_ptr->from_solver.max_modules_per_partition; i++) {
                    if (i <  fl_ptr->alloc[k].num_hw_tasks_in_part) {
                        src = "Bitstreams/config_" + std::to_string(i) + "_pblock_slot_" + std::to_string(k) + "_partial.bin";
                        //dest = "fred/dart_fred/bits/p"+ std::to_string(k) + "/" + rm_list[fl_ptr->alloc[k].task_id[i]].rm_tag + "_s" +  std::to_string(i) + ".bin";
                        ip_id = fl_ptr->alloc[k].task_id[i];
                        dest = "fred/dart_fred/bits/p"+ std::to_string(k) + "/" + config["dart"]["hw_ips"][ip_id]["ip_name"].as<string>() + "_s0.bin";
                        // if (fs::exists(fs::path(dest))){
                        //     fs::remove(fs::path(dest));
                        // }
                        fs::copy_file(src, dest,fs::copy_options::update_existing);
                    }
                }
            }

            /*Copy one of the static bitstreams to FRED*/
            // if (fs::exists(fs::path("fred/dart_fred/bits/static.bin"))){
            //     fs::remove(fs::path("fred/dart_fred/bits/static.bin"));
            // }
            fs::copy_file("Bitstreams/config_0.bin", "fred/dart_fred/bits/static.bin",fs::copy_options::update_existing);

            write_fred_arch.close();
            write_fred_hw.close();

        #else
            /* Create the arch.csv and hw_tasks.csv files for FRED */
            for(k = 0; k < num_rm_partitions; k++) {
                //write_fred_arch << "p"<<k << ", "  << alloc[k].num_hw_tasks_in_part << "\n";
                write_fred_arch << "p"<<k << ", "  << "1\n";
            }

            for(k = 0; k < num_rm_partitions; k++) {
                for(i = 0; i < config["dart"]["partitions"][k]["hw_ips"].size(); i++) {
                    // commented since it' s always true
                    //if (i < alloc[k].num_hw_tasks_in_part) {
                        //write_fred_hw << rm_list[alloc[k].rm_id[i]].rm_tag <<", " <<bitstream_id << ", p"<<k<<", dart_fred/bits, ";
                        write_fred_hw << config["dart"]["partitions"][k]["hw_ips"][i]["ip_name"].as<string>() <<", " <<bitstream_id << 
                            ", " << config["dart"]["partitions"][k]["hw_ips"][i]["timeout"].as<uint>() << ", p"<<k<<", dart_fred/bits, ";
                        bitstream_id++;
                        n_buffers = config["dart"]["partitions"][k]["hw_ips"][i]["buffers"].size();
                        for(j = 0; j < n_buffers; j++) {
                            write_fred_hw << config["dart"]["partitions"][k]["hw_ips"][i]["buffers"][j].as<int>();
                            if (j < (n_buffers-1)){
                                write_fred_hw << ", ";
                            }
                        }
                        write_fred_hw << "\n";
                    //}
                }
            }
            /*
            for(k = 0; k < num_rm_partitions; k++) {
                for(i = 0; i < max_modules_in_partition; i++) {
                    if (i < alloc[k].num_hw_tasks_in_part) {
                        write_fred_hw << rm_list[alloc[k].rm_id[i]].rm_tag <<", " <<bitstream_id << ", p"<<k<<", dart_fred/bits, " <<fred_input_buff_size <<", " << fred_output_buff_size <<"\n";
                        bitstream_id++;
                    }
                }
            }
            */
        
            /* Create the FRED bitstream partition directories */
            for(k = 0; k < num_rm_partitions; k++) {
                str = "fred/dart_fred/bits/p"+ std::to_string(k);
                fs::create_directories(str);
            }

            /* Copy the partial bitstreams */
            for(k = 0; k < num_rm_partitions; k++) {
                //for(i = 0; i < max_modules_in_partition; i++) {
                for(i = 0; i < config["dart"]["partitions"][k]["hw_ips"].size(); i++) {
                    //if (i < alloc[k].num_hw_tasks_in_part) {
                        src = "Bitstreams/config_" + std::to_string(i) + "_pblock_slot_" + std::to_string(k) + "_partial.bin"; 
                        //dest = "fred/dart_fred/bits/p"+ std::to_string(k) + "/" + rm_list[alloc[k].rm_id[i]].rm_tag + "_s" +  std::to_string(i) + ".bin";
                        dest = "fred/dart_fred/bits/p"+ std::to_string(k) + "/" + config["dart"]["partitions"][k]["hw_ips"][i]["ip_name"].as<string>() + "_s0.bin";
                        // if (fs::exists(fs::path(dest))){
                        //     fs::remove(fs::path(dest));
                        // }
                        fs::copy_file(src, dest,fs::copy_options::update_existing);
                    //}
                }
            }

            /*Copy one of the static bitstreams to FRED*/
            // if (fs::exists(fs::path("fred/dart_fred/bits/static.bin"))){
            //     fs::remove(fs::path("fred/dart_fred/bits/static.bin"));
            // }
            fs::copy_file("Bitstreams/config_0.bin", "fred/dart_fred/bits/static.bin",fs::copy_options::update_existing);

            write_fred_arch.close();
            write_fred_hw.close();
        #endif
    }catch (std::system_error & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not create the FRED files" << endl;
        exit(EXIT_FAILURE);
    } 
}

void pr_tool::prep_proj_directory()
{
    int status, i,j;
    
    try{
        cout << "PR_TOOL: creating project directory "<<endl;
        fs::create_directories(Src_path);
        fs::create_directories(Src_path / fs::path("project"));
        fs::create_directories(Src_path / fs::path("constraints"));
        fs::create_directories(Src_path / fs::path("cores"));
        fs::create_directories(Src_path / fs::path("netlist"));
        fs::create_directories(Src_path / fs::path("ip_repo"));
        fs::create_directories(Project_dir / fs::path("Synth"));
        fs::create_directories(Project_dir / fs::path("Synth") / fs::path("Static"));
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
        string ip_name;
#ifdef WITH_PARTITIONING
        for(i = 0; i < num_rm_modules; i++){
            ip_name = config["dart"]["hw_ips"][i]["ip_name"].as<std::string>();
            std::string str = Src_path / fs::path("cores") / ip_name;
            if (!fs::exists(str)) { 
                fs::create_directories(str);
                fs::copy(fs::path(dart_ip_path) / ip_name, str, fs::copy_options::recursive); 
            }
        } 
#else
        for(i = 0; i < num_rm_partitions; i++) {
            for(j = 0; j < config["dart"]["partitions"][i]["hw_ips"].size(); j++) {
                ip_name = config["dart"]["partitions"][i]["hw_ips"][j]["ip_name"].as<std::string>();
                std::string str = Src_path / fs::path("cores") / ip_name;
                if (!fs::exists(str)) { 
                    fs::create_directories(str);
                    fs::copy(fs::path(dart_ip_path) / ip_name, str, fs::copy_options::recursive); 
                }
            }
        }
#endif 
        fs::path dir_source(dart_path);
        dir_source /= fs::path("tools") / fs::path("Tcl");
        if (fs::exists(fs::path(tcl_project))) { 
            fs::remove_all(tcl_project);
        }
        fs::copy(dir_source, tcl_project, fs::copy_options::recursive); 
        dir_source = dart_path / fs::path("tools") / fs::path("start_vivado");
        //if (!fs::exists(fs::path(Project_dir) / fs::path("start_vivado"))) { 
            fs::copy(dir_source, Project_dir, fs::copy_options::update_existing); 
        //}
        dir_source = dart_path / fs::path("tools") / fs::path("acc_bbox_ip");
        if (fs::exists(fs::path(ip_repo_path))) { 
            fs::remove_all(ip_repo_path);
        }
        fs::copy(dir_source, ip_repo_path, fs::copy_options::recursive);
    }catch (std::system_error & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not create the DART project" << endl;
        exit(EXIT_FAILURE);
    } 
}

string pr_tool::module_attributes(string ip_name, string module_name){
    stringstream s;
    s << "add_module " << ip_name <<endl;
    s << "set_attribute module " <<ip_name << " moduleName\t" << module_name  <<endl;
    s << "set_attribute module " <<ip_name << " prj \t" << "$prjDir/" << ip_name <<".prj" <<endl;                   
    s << "set_attribute module " <<ip_name << " synth \t" << "${run.rmSynth}" <<endl;
    // complex IPs might internally use other IPs. This part checks whether this is the case for this IP and 
    // it adds the list of xci files into the output files
    s << "set xci_files [::fileutil::findByPattern ${coreDir}/" <<ip_name <<" -glob {*.xci}]" <<endl;
    s << "if {[llength $xci_files] > 0} {" <<endl;
    s << "    set_attribute module " << ip_name << " ip $xci_files" <<endl;
    s << "}" <<endl;
    s <<endl;

    return s.str();
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


     write_synth_tcl << "package require fileutil" <<endl <<endl;
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
#elif FPGA_ZCU_102
    write_synth_tcl << "set part xczu9eg-ffvb1156-2-e" <<endl;
#elif FPGA_US_96
    write_synth_tcl << "set part xczu3eg-sbva484-1-i" <<endl;
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
     write_synth_tcl<<"####################################################################" <<endl;

#ifdef WITH_PARTITIONING
    if (re_synthesis_after_wrap == 1) {
        for(i = 0; i < fl_ptr->from_solver.num_partition; i++) {
            for(k = 0; k <  fl_ptr->alloc[i].num_hw_tasks_in_part; k++) {          
                //tag = rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag;
                tag = config["dart"]["hw_ips"][fl_ptr->alloc[i].task_id[k]]["ip_name"].as<std::string>();
                write_synth_tcl << module_attributes(tag,string(wrapper_top_name + "_" + to_string(i)));
            }
        }
    }
    
    else {     
         for(i = 0; i < num_rm_modules; i++) {
            tag = config["dart"]["hw_ips"][i]["ip_name"].as<std::string>();
            top_mod = config["dart"]["hw_ips"][i]["top_name"].as<std::string>();
            //write_synth_tcl << module_attributes(rm_list[i].rm_tag,rm_list[i].top_module);
            write_synth_tcl << module_attributes(tag,top_mod);
         }
     }
#else
    //  for(i = 0; i < num_rm_modules; i++) {
    //      write_synth_tcl << module_attributes(rm_list[i].rm_tag,string(wrapper_top_name + "_" + to_string(rm_list[i].partition_id)));
    //  }
    for(i = 0; i < num_rm_partitions; i++) {
        for(j = 0; j < config["dart"]["partitions"][i]["hw_ips"].size(); j++) {
            write_synth_tcl << module_attributes(
                config["dart"]["partitions"][i]["hw_ips"][j]["ip_name"].as<string>(),
                string(wrapper_top_name + "_" + to_string(i))
            );
        }
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
            cerr << "ERROR: '" << dir.string() << "' directory not found.\n";
            exit(1);
        }
        
        dir = fs::path("project");
        if (!fs::exists(dir) || !fs::is_directory(dir)){
            cerr << "ERROR: '" << dir.string() << "' directory not found.\n";
            exit(1);
        }

        // iterates over every dirs in 'cores'
        for (const auto & entry : fs::directory_iterator("cores")){
            if (!fs::is_directory(entry.path())){
                cerr << "ERROR: expecting only directories with IPs inside the 'cores' dir.\n";
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
                cerr << "ERROR: '" << vhd_path.string() << "' directory not found.\n";
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

            // some complex IPs use internal IPs themselves, but the dir is optional
            vhd_path = fs::path("cores") / ip_name / fs::path("hdl") / fs::path("ip");
            if (fs::exists(vhd_path) && fs::is_directory(vhd_path)){
                // write all vhd file names into the prj file
                int vhd_file_cnt=0;
                for (const auto & entry2 : fs::directory_iterator(vhd_path)){
                    if (fs::is_regular_file(entry2) && (fs::path(entry2).extension() == ".vhd")){
                        prj_file << "vhdl xil_defaultlib ./Sources/" + entry2.path().string() +"\n";
                        vhd_file_cnt++;
                    }
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
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not create Vivado project" << endl;
        exit(EXIT_FAILURE);
    }      
}

void pr_tool::run_vivado(std::string synth_script)
{ 
    int ret_code;
    chdir(("cd " + Project_dir).c_str());
    //fs::current_path(Src_path);
    //std::system(("python3.6  load_prj.py"));
    // the python file execution has been replaced by create_vivado_project()
    fs::current_path(Project_dir);
    fs::path bash_script(Project_dir);
    bash_script /= fs::path("start_vivado");
    if (!fs::is_regular_file(bash_script)){
        cerr << "ERROR: " << bash_script.string() << " not found\n";
        exit(1);
    }
    fs::path vivado_script(synth_script);
    if (!fs::is_regular_file(vivado_script)){
        cerr << "ERROR: " << vivado_script.string() << " not found\n";
        exit(1);
    }
    // run vivado
    try{
        ret_code = std::system((bash_script.string() + " " + vivado_script.string()).c_str());
        if (ret_code != EXIT_SUCCESS) {
            cerr << "ERROR: Vivado synthesis error" << endl;
            exit(EXIT_FAILURE);
        }
    }
    catch (std::system_error & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not run Vivado" << endl;
        exit(EXIT_FAILURE);
    }   
}

void pr_tool::parse_synthesis_report()
{
    unsigned long i, k, j = 0;
#if defined(FPGA_ZCU_102) || defined (FPGA_US_96)
    string lut = "CLB LUTs*";
#else
    string lut = "Slice LUTs*";
#endif    
    string dsp_str = "DSPs";
    string bram_str = "Block RAM Tile    |";
    fs::path log_file_name;
    //vector<slot> extracted_res = vector<slot>(num_rm_modules);
   
    cout << "PR_TOOL: Parsing Synth utilization report "  <<endl;
    
#ifdef WITH_PARTITIONING
    unsigned long clb, bram, dsp;    
    for(i = 0; i < num_rm_modules; i++) {
        string line, word;
        //cout <<"PR_TOOL:filename is " << Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
        //              rm_list[i].top_module + "_utilization_synth.rpt" <<endl;
        cout <<"PR_TOOL:filename is " << Project_dir + "/Synth/" + config["dart"]["hw_ips"][i]["ip_name"].as<std::string>() + "/" + 
                      config["dart"]["hw_ips"][i]["top_name"].as<std::string>() + "_utilization_synth.rpt" <<endl;

        //ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
        //              rm_list[i].top_module + "_utilization_synth.rpt");
        log_file_name = Project_dir + "/Synth/" + config["dart"]["hw_ips"][i]["ip_name"].as<std::string>() + "/" + 
                      config["dart"]["hw_ips"][i]["top_name"].as<std::string>() + "_utilization_synth.rpt";
        if (!fs::exists(log_file_name)){
            cerr << "ERROR: log file " << log_file_name << " not found!\n";
            exit(EXIT_FAILURE);
        }
        ifstream file (log_file_name);

        k = 0;
        while (getline(file, line)) {
            if(line.find(lut) != string::npos) {
                stringstream iss(line);
                while(iss >> word) {
                    k++;
                    if(k == 5){
                        //extracted_res[i].clb= (unsigned long)((std::stoul(word) / 8));
                        clb = (unsigned long)((std::stoul(word) / 8));
                        config["dart"]["hw_ips"][i]["CLBs"] = clb;
                        cout << " clb " << clb <<endl;
                    }
                }
                k = 0;
            }

            if(line.find(bram_str) != string::npos) {
                stringstream iss(line);
                while(iss >> word) {
                    k++;
                    if(k == 6) {
                        //extracted_res[i].bram = std::stoul(word);
                        bram = std::stoul(word);
                        config["dart"]["hw_ips"][i]["BRAMs"] = bram;
                        cout <<" bram " <<  bram <<endl;
                    }
                }
                k = 0;
            }

            if(line.find(dsp_str) != string::npos){
                stringstream iss(line);
                while(iss >> word) {
                    k++;
                    if(k == 4){
                        //extracted_res[i].dsp = std::stoul(word);
                        dsp = std::stoul(word);
                        config["dart"]["hw_ips"][i]["DSPs"] = bram;
                        cout << " dsp " <<  dsp <<endl;
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
        for(i = 0; i < config["dart"]["partitions"][j]["hw_ips"].size(); i++) {
        //for(i = 0; i < num_rm_modules; i++) {
            string line, word;

            //if(rm_list[i].partition_id == j) {
                k = 0;
                //cout <<"PR_TOOL:filename is " << Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                //              wrapper_top_name + "_" + to_string(rm_list[i].partition_id) + "_utilization_synth.rpt" <<endl;
                cout <<"PR_TOOL:filename is " << Project_dir + "/Synth/" + config["dart"]["partitions"][j]["hw_ips"][i]["ip_name"].as<string>() + "/" + 
                              wrapper_top_name + "_" + to_string(j) + "_utilization_synth.rpt" <<endl;

                //ifstream file (Project_dir + "/Synth/" + rm_list[i].rm_tag + "/" + 
                //              wrapper_top_name + "_" + to_string(rm_list[i].partition_id) + "_utilization_synth.rpt");
                log_file_name = Project_dir + "/Synth/" + config["dart"]["partitions"][j]["hw_ips"][i]["ip_name"].as<string>() + "/" + 
                              wrapper_top_name + "_" + to_string(j) + "_utilization_synth.rpt";
                if (!fs::exists(log_file_name)){
                    cerr << "ERROR: log file " << log_file_name << " not found!\n";
                    exit(EXIT_FAILURE);
                }
                ifstream file (log_file_name);

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

                    if(line.find(bram_str) != string::npos) {
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

                    if(line.find(dsp_str) != string::npos){
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
            //}
        }
        /*
        extracted_res[j].clb = max_clb;
        extracted_res[j].bram = max_bram;
        extracted_res[j].dsp = max_dsp;
        */
        config["dart"]["partitions"][j]["CLBs"] = max_clb;
        config["dart"]["partitions"][j]["BRAMs"] = max_bram;
        config["dart"]["partitions"][j]["DSPs"] = max_dsp;
    }

#endif    
    ofstream write_flora_input;
    write_flora_input.open(Project_dir +"/flora_input.csv");
    int clbs, brams, dsps;
#ifdef WITH_PARTITIONING         
    for(i = 0; i < num_rm_modules; i++){
        clbs = config["dart"]["hw_ips"][i]["CLBs"].as<int>();
        brams = config["dart"]["hw_ips"][i]["BRAMs"].as<int>();
        dsps = config["dart"]["hw_ips"][i]["DSPs"].as<int>();
#else
    for(i = 0; i < num_rm_partitions; i++){
        clbs = config["dart"]["partitions"][i]["CLBs"].as<int>();
        brams = config["dart"]["partitions"][i]["BRAMs"].as<int>();
        dsps = config["dart"]["partitions"][i]["DSPs"].as<int>();
#endif
        //write_flora_input <<extracted_res[i].clb + CLB_MARGIN <<"," ; 
        write_flora_input << clbs + CLB_MARGIN <<"," ; 
        
        if(brams > 0)
            //write_flora_input << extracted_res[i].bram + BRAM_MARGIN <<",";
            write_flora_input << brams + BRAM_MARGIN <<",";
        else
            //write_flora_input << extracted_res[i].bram << "," ;
            write_flora_input << brams << "," ;

        if(dsps > 0)
            //write_flora_input << extracted_res[i].dsp + DSP_MARGIN <<"," ;
            write_flora_input << dsps + DSP_MARGIN <<"," ;
        else
            //write_flora_input << extracted_res[i].dsp <<"," ;
            write_flora_input << dsps <<"," ;

#ifdef WITH_PARTITIONING
        //write_flora_input << HW_WCET[i] << "," <<slacks[i] <<"," ;
        //write_flora_input << HW_WCET[i] << "," <<slacks[i]  ;
        write_flora_input << 
            config["dart"]["hw_ips"][i]["wcet"].as<int>() << "," << 
            config["dart"]["hw_ips"][i]["slack_time"].as<int>();
#endif
        //write_flora_input << rm_list[i].rm_tag <<endl;
        write_flora_input <<endl;
    }
        
        write_flora_input.close();
}

void pr_tool::generate_impl_tcl(flora *fl_ptr)
{ 
    unsigned long i,j,k, partition_ptr, temp_index = 0;
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
#elif FPGA_ZCU_102
    write_impl_tcl << "set part xczu9eg-ffvb1156-2-e" <<endl;
#elif FPGA_US_96
    write_impl_tcl << "set part xczu3eg-sbva484-1-i" <<endl;
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
     write_impl_tcl<<"####################################################################" <<endl;

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
                //tag = rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag;
                tag = config["dart"]["hw_ips"][fl_ptr->alloc[i].task_id[k]]["ip_name"].as<std::string>();
                write_impl_tcl << "add_module " << tag <<endl;
                write_impl_tcl << "set_attribute module " <<tag << " moduleName\t" << wrapper_top_name << "_" << i <<endl;
                
           }
        }
//    }
#else
/*
    for(i = 0; i < num_rm_modules; i++) {
        write_impl_tcl << "add_module " << rm_list[i].rm_tag <<endl;
        write_impl_tcl << "set_attribute module "  << rm_list[i].rm_tag <<  " moduleName\t" << wrapper_top_name << "_" << rm_list[i].partition_id <<endl;
        write_impl_tcl <<endl;
     }
     */
    for(i = 0; i < num_rm_partitions; i++) {
        for(j = 0; j < config["dart"]["partitions"][i]["hw_ips"].size(); j++) {
            write_impl_tcl << "add_module " << config["dart"]["partitions"][i]["hw_ips"][j]["ip_name"].as<string>() <<endl;
            write_impl_tcl << "set_attribute module "  << config["dart"]["partitions"][i]["hw_ips"][j]["ip_name"].as<string>() 
                <<  " moduleName\t" << wrapper_top_name << "_" << i <<endl;
            write_impl_tcl <<endl;
        }
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
                //write_impl_tcl <<"[list " << rm_list[fl_ptr->alloc[partition_ptr].task_id[temp_index]].rm_tag 
                //   <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
                write_impl_tcl <<"[list " << config["dart"]["hw_ips"][fl_ptr->alloc[partition_ptr].task_id[temp_index]]["ip_name"].as<std::string>()
                   <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
            }

            else {
                write_impl_tcl <<"\t \t \t \t \t";
                //write_impl_tcl <<"[list " << rm_list[fl_ptr->alloc[partition_ptr].task_id[0]].rm_tag
                //               <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" import] \\" <<endl;
                write_impl_tcl <<"[list " << config["dart"]["hw_ips"][fl_ptr->alloc[partition_ptr].task_id[0]]["ip_name"].as<std::string>()
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
    unsigned idx, max_modules_in_partition=0;
    vector<unsigned> rm_per_part;
    for(i=0;i<num_rm_partitions;i++){
        rm_per_part.push_back(config["dart"]["partitions"][i]["hw_ips"].size());
        if(max_modules_in_partition < config["dart"]["partitions"][i]["hw_ips"].size())
            max_modules_in_partition = config["dart"]["partitions"][i]["hw_ips"].size();
    }
     
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
            if (rm_per_part[partition_ptr]>0){
                rm_per_part[partition_ptr]--;
            // if(alloc[partition_ptr].num_modules_in_partition > 0) {
            //     alloc[partition_ptr].num_modules_in_partition--;
                //idx = alloc[partition_ptr].rm_id[temp_index];
                // write_impl_tcl <<"\t \t \t \t \t";
                // write_impl_tcl <<"[list " << rm_list[alloc[partition_ptr].rm_id[temp_index]].rm_tag 
                //    <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << config["dart"]["partitions"][partition_ptr]["hw_ips"][temp_index]["ip_name"].as<std::string>() <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" implement] \\" <<endl;
            }
            else {
                //idx = alloc[partition_ptr].rm_id[0];
                // write_impl_tcl <<"\t \t \t \t \t";
                // write_impl_tcl <<"[list " << rm_list[alloc[partition_ptr].rm_id[0]].rm_tag 
                //                <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" import] \\" <<endl; 
                write_impl_tcl <<"\t \t \t \t \t";
                write_impl_tcl <<"[list " << config["dart"]["partitions"][partition_ptr]["hw_ips"][0]["ip_name"].as<std::string>()  <<"\t " << fl_ptr->cell_name[partition_ptr]  <<" import] \\" <<endl; 
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
    //Project_dir = input_pr->path_to_output;

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
    write_static_tcl << "set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project] " <<endl;
#elif FPGA_ZYNQ
    write_static_tcl << "create_project dart_project -force " << static_dir << " -part xc7z010clg400-1 " <<endl;
    write_static_tcl << "set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project] " <<endl;
#elif FPGA_ZCU_102
     write_static_tcl << "create_project dart_project -force " << static_dir << " -part xczu9eg-ffvb1156-2-e" <<endl;
     write_static_tcl << "set_property board_part xilinx.com:zcu102:part0:3.4 [current_project] " <<endl;
#elif FPGA_US_96
     //write_static_tcl << "create_project dart_project -force " << static_dir << " -part xczu3eg-sbva484-1-e" <<endl;
     write_static_tcl << "create_project dart_project -force " << static_dir << " -part xczu3eg-sbva484-1-i" <<endl;
     //write_static_tcl << "set_property board_part em.avnet.com:ultra96v2:part0:1.2 [current_project]" <<endl;
     write_static_tcl << "set_property board_part avnet.com:ultra96v2:part0:1.2 [current_project]" <<endl;
#else
    write_static_tcl << "create_project dart_project -force " << static_dir << " -part xc7z010clg400-1 " <<endl;
    write_static_tcl << "set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project] " <<endl;
#endif

    write_static_tcl << "set_property  ip_repo_paths "<<ip_repo_path<<" [current_project]" <<endl;
    write_static_tcl << "update_ip_catalog " <<endl;
    write_static_tcl << "create_bd_design \"dart\" " <<endl;
    write_static_tcl << "update_compile_order -fileset sources_1 " <<endl;
#if defined(FPGA_PYNQ) || defined(FPGA_ZYNQ)
    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0" <<endl;
    write_static_tcl << "endgroup " <<endl;
#elif defined (FPGA_ZCU_102) || defined (FPGA_US_96)
    write_static_tcl << "startgroup " <<endl;
    //write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.2 zynq_ultra_ps_e_0" <<endl; //TODO: PS must be templated
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.3 zynq_ultra_ps_e_0" <<endl; // TODO: this works with 2020.2. Perhaps the IP version is different for older versions
    write_static_tcl << "endgroup " <<endl;
#endif

    // TODO: apply ILA per partition and read it from the YAML
    //bool use_ila = false;
   
    //create the bbox instance of the accelerator IPs and the decouplers (two decouplers for each acc)
    string vivado_version_str(config["vivado_version_str"].as<std::string>());
    Version vivado_version(vivado_version_str);
    Version min_vivado_version("2020.1.0.0");
    for(i=0, j=0; i < num_partitions; i++, j++) {
        write_static_tcl << "startgroup " <<endl;
        write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:hls:acc:1.0 acc_" <<std::to_string(i) <<endl;
        write_static_tcl << "endgroup " <<endl;
        write_static_tcl << "startgroup " <<endl;
        //if (vivado_version == 1)
        if (vivado_version<min_vivado_version)
            write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:pr_decoupler:1.0 pr_decoupler_"<<std::to_string(j) <<endl; //for Vivado 2019.2 and below
        else
            write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 pr_decoupler_"<<std::to_string(j) <<endl;
        write_static_tcl << "endgroup " <<endl;
        /*
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
                            */
   
        write_static_tcl << "set_property -dict [list CONFIG.ALL_PARAMS {HAS_AXI_LITE 1 HAS_SIGNAL_CONTROL 0 INTF {acc_ctrl "
                            "{ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 PROTOCOL axi4lite SIGNALS {ARVALID {PRESENT 1} "
                            "ARREADY {PRESENT 1} AWVALID {PRESENT 1} AWREADY {PRESENT 1} BVALID {PRESENT 1} BREADY {PRESENT 1} "
                            "RVALID {PRESENT 1} RREADY {PRESENT 1} WVALID {PRESENT 1} WREADY {PRESENT 1} AWADDR {PRESENT 1} "
                            "AWLEN {PRESENT 0} AWSIZE {PRESENT 0} AWBURST {PRESENT 0} AWLOCK {PRESENT 0} AWCACHE {PRESENT 0} "
                            "AWPROT {PRESENT 1} WDATA {PRESENT 1} WSTRB {PRESENT 1} WLAST {PRESENT 0} BRESP {PRESENT 1} "
                            "ARADDR {PRESENT 1} ARLEN {PRESENT 0} ARSIZE {PRESENT 0} ARBURST {PRESENT 0} ARLOCK {PRESENT 0} "
                            "ARCACHE {PRESENT 0} ARPROT {PRESENT 1} RDATA {PRESENT 1} RRESP {PRESENT 1} RLAST {PRESENT 0}} "
                            "MODE slave} rp_reset {ID 1 VLNV xilinx.com:signal:reset_rtl:1.0}}} CONFIG.GUI_HAS_AXI_LITE {1} "
                            "CONFIG.GUI_HAS_SIGNAL_CONTROL {0} CONFIG.GUI_SELECT_INTERFACE {1} CONFIG.GUI_INTERFACE_NAME {rp_reset} "
                            "CONFIG.GUI_SELECT_VLNV {xilinx.com:signal:reset_rtl:1.0} CONFIG.GUI_INTERFACE_PROTOCOL {none} "
                            "CONFIG.GUI_SELECT_MODE {master} CONFIG.GUI_SIGNAL_SELECT_0 {RST} CONFIG.GUI_SIGNAL_SELECT_1 {-1} "
                            "CONFIG.GUI_SIGNAL_SELECT_2 {-1} CONFIG.GUI_SIGNAL_SELECT_3 {-1} CONFIG.GUI_SIGNAL_SELECT_4 {-1} "
                            "CONFIG.GUI_SIGNAL_SELECT_5 {-1} CONFIG.GUI_SIGNAL_SELECT_6 {-1} CONFIG.GUI_SIGNAL_SELECT_7 {-1} "
                            "CONFIG.GUI_SIGNAL_SELECT_8 {-1} CONFIG.GUI_SIGNAL_SELECT_9 {-1} CONFIG.GUI_SIGNAL_DECOUPLED_0 {true} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_1 {false} CONFIG.GUI_SIGNAL_DECOUPLED_2 {false} CONFIG.GUI_SIGNAL_DECOUPLED_3 {false} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_4 {false} CONFIG.GUI_SIGNAL_DECOUPLED_5 {false} CONFIG.GUI_SIGNAL_DECOUPLED_6 {false} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_7 {false} CONFIG.GUI_SIGNAL_DECOUPLED_8 {false} CONFIG.GUI_SIGNAL_DECOUPLED_9 {false} "
                            "CONFIG.GUI_SIGNAL_PRESENT_0 {true} CONFIG.GUI_SIGNAL_PRESENT_1 {false} CONFIG.GUI_SIGNAL_PRESENT_2 {false} CONFIG.GUI_SIGNAL_PRESENT_3 "
                            "{false} CONFIG.GUI_SIGNAL_PRESENT_4 {false} CONFIG.GUI_SIGNAL_PRESENT_5 {false} CONFIG.GUI_SIGNAL_PRESENT_6 {false} "
                            "CONFIG.GUI_SIGNAL_PRESENT_7 {false} CONFIG.GUI_SIGNAL_PRESENT_8 {false} CONFIG.GUI_SIGNAL_PRESENT_9 {false}] [get_bd_cells pr_decoupler_" << std::to_string(j)<< "]" <<endl;

   
        j++; 
        write_static_tcl << "startgroup" << endl;
        if (vivado_version<min_vivado_version)
        //if (vivado_version == 1)
            write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:pr_decoupler:1.0 pr_decoupler_"<<std::to_string(j) << endl; //for Vivado 2019.2 and below
        else
            write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 pr_decoupler_"<<std::to_string(j) << endl;
        write_static_tcl << "endgroup" << endl;
    /*
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
    */
    
        write_static_tcl << "set_property -dict [list CONFIG.ALL_PARAMS {HAS_SIGNAL_STATUS 0 INTF {acc_data {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0} "
                            "acc_interrupt {ID 1 VLNV xilinx.com:signal:interrupt_rtl:1.0}}} CONFIG.GUI_HAS_SIGNAL_STATUS {0} CONFIG.GUI_SELECT_INTERFACE {1} "
                            "CONFIG.GUI_INTERFACE_NAME {acc_interrupt} CONFIG.GUI_SELECT_VLNV {xilinx.com:signal:interrupt_rtl:1.0} CONFIG.GUI_INTERFACE_PROTOCOL {none} "
                            "CONFIG.GUI_SIGNAL_SELECT_0 {INTERRUPT} CONFIG.GUI_SIGNAL_SELECT_1 {-1} CONFIG.GUI_SIGNAL_SELECT_2 {-1} CONFIG.GUI_SIGNAL_SELECT_3 {-1} "
                            "CONFIG.GUI_SIGNAL_SELECT_4 {-1} CONFIG.GUI_SIGNAL_SELECT_5 {-1} CONFIG.GUI_SIGNAL_SELECT_6 {-1} CONFIG.GUI_SIGNAL_SELECT_7 {-1} "
                            "CONFIG.GUI_SIGNAL_SELECT_8 {-1} CONFIG.GUI_SIGNAL_SELECT_9 {-1} CONFIG.GUI_SIGNAL_DECOUPLED_0 {true} CONFIG.GUI_SIGNAL_DECOUPLED_1 {false} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_2 {false} CONFIG.GUI_SIGNAL_DECOUPLED_3 {false} CONFIG.GUI_SIGNAL_DECOUPLED_4 {false} CONFIG.GUI_SIGNAL_DECOUPLED_5 {false} "
                            "CONFIG.GUI_SIGNAL_DECOUPLED_6 {false} CONFIG.GUI_SIGNAL_DECOUPLED_7 {false} CONFIG.GUI_SIGNAL_DECOUPLED_8 {false} CONFIG.GUI_SIGNAL_DECOUPLED_9 {false} "
                            "CONFIG.GUI_SIGNAL_PRESENT_0 {true} CONFIG.GUI_SIGNAL_PRESENT_1 {false} CONFIG.GUI_SIGNAL_PRESENT_2 {false} CONFIG.GUI_SIGNAL_PRESENT_3 {false} "
                            "CONFIG.GUI_SIGNAL_PRESENT_4 {false} CONFIG.GUI_SIGNAL_PRESENT_5 {false} CONFIG.GUI_SIGNAL_PRESENT_6 {false} CONFIG.GUI_SIGNAL_PRESENT_7 {false} "
                            "CONFIG.GUI_SIGNAL_PRESENT_8 {false} CONFIG.GUI_SIGNAL_PRESENT_9 {false}] [get_bd_cells pr_decoupler_"<< std::to_string(j) << "]" <<endl;

        // decided whether the ILA module must be inserted for this partition
        if(config["dart"]["partitions"][i]["debug"]){
            // get the ILA buffer size. If not defined, use the defaut value
            int ila_buffer_depth = 1024;
            if(config["dart"]["partitions"][i]["debug"]["data_depth"]){
                ila_buffer_depth = config["dart"]["partitions"][i]["debug"]["data_depth"].as<int>();
            }
            // add one ILA per reconfig region
            write_static_tcl << "# Create instance: system_ila_"<< std::to_string(i) <<", and set properties " <<endl;
            write_static_tcl << "set system_ila_"<< std::to_string(i) <<" [ create_bd_cell -type ip -vlnv xilinx.com:ip:system_ila:1.1 system_ila_"<< std::to_string(i) <<" ] " <<endl;
            // TODO: it might be interesting to enable changing CONFIG.C_DATA_DEPTH {XXXX}"
            //write_static_tcl << "set_property -dict [ list CONFIG.C_BRAM_CNT {12} CONFIG.C_DATA_DEPTH {1024} "
            write_static_tcl << "set_property -dict [ list CONFIG.C_BRAM_CNT {23.5} CONFIG.C_DATA_DEPTH {" << ila_buffer_depth << "} "
                                "  CONFIG.C_MON_TYPE {MIX} CONFIG.C_NUM_MONITOR_SLOTS {2} CONFIG.C_SLOT_0_APC_EN {0}"
                                "  CONFIG.C_SLOT_0_AXI_AR_SEL_DATA {1} CONFIG.C_SLOT_0_AXI_AR_SEL_TRIG {1} CONFIG.C_SLOT_0_AXI_AW_SEL_DATA {1}"
                                "  CONFIG.C_SLOT_0_AXI_AW_SEL_TRIG {1} CONFIG.C_SLOT_0_AXI_B_SEL_DATA {1} CONFIG.C_SLOT_0_AXI_B_SEL_TRIG {1}"
                                "  CONFIG.C_SLOT_0_AXI_R_SEL_DATA {1} CONFIG.C_SLOT_0_AXI_R_SEL_TRIG {1} CONFIG.C_SLOT_0_AXI_W_SEL_DATA {1}"
                                "  CONFIG.C_SLOT_0_AXI_W_SEL_TRIG {1}"
                                "  CONFIG.C_SLOT_0_INTF_TYPE {xilinx.com:interface:aximm_rtl:1.0}"
                                "  CONFIG.C_SLOT_1_APC_EN {0}"
                                "  CONFIG.C_SLOT_1_AXI_AR_SEL_DATA {1} CONFIG.C_SLOT_1_AXI_AR_SEL_TRIG {1} CONFIG.C_SLOT_1_AXI_AW_SEL_DATA {1}"
                                "  CONFIG.C_SLOT_1_AXI_AW_SEL_TRIG {1} CONFIG.C_SLOT_1_AXI_B_SEL_DATA {1} CONFIG.C_SLOT_1_AXI_B_SEL_TRIG {1}"
                                "  CONFIG.C_SLOT_1_AXI_R_SEL_DATA {1} CONFIG.C_SLOT_1_AXI_R_SEL_TRIG {1} CONFIG.C_SLOT_1_AXI_W_SEL_DATA {1}"
                                "  CONFIG.C_SLOT_1_AXI_W_SEL_TRIG {1}"
                                "  CONFIG.C_SLOT_1_INTF_TYPE {xilinx.com:interface:aximm_rtl:1.0}"
                                "] [get_bd_cells system_ila_"<< std::to_string(i) << "]" << endl;
        }
    }
  
    //connect PS to DDR and Fixed IO
#ifdef FPGA_PYNQ
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external \"FIXED_IO, DDR\" "
                        "apply_board_preset \"1\" Master \"Disable\" Slave \"Disable\" }  [get_bd_cells processing_system7_0]" <<endl;
    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0] " <<endl; //TODO: number of HPO should be changed based on number of acc
    write_static_tcl << "endgroup " <<endl;
#elif defined (FPGA_ZCU_102) || defined (FPGA_US_96)
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:zynq_ultra_ps_e -config {apply_board_preset \"1\" }  [get_bd_cells zynq_ultra_ps_e_0]" <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.PSU__USE__S_AXI_GP0 {1}] [get_bd_cells zynq_ultra_ps_e_0] " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.PSU__USE__M_AXI_GP1 {0}] [get_bd_cells zynq_ultra_ps_e_0] " <<endl;
#endif

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
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins acc_"<<std::to_string(i)<<"/s_axi_ctrl_bus] [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/rp_acc_ctrl]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_ctrl] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M0"<<std::to_string(j)<<"_AXI]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M0"<<std::to_string(j+1)<<"_AXI]" <<endl;
#ifdef FPGA_PYNQ        
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]" <<endl; 
        write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<std::to_string(i)<<"/ap_clk] [get_bd_pins processing_system7_0/FCLK_CLK0]" <<endl;
#elif defined(FPGA_ZCU_102) || defined(FPGA_US_96)
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins zynq_ultra_ps_e_0/M_AXI_HPM0_FPD]" <<endl; 
        write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<std::to_string(i)<<"/ap_clk] [get_bd_pins zynq_ultra_ps_e_0/pl_clk0]" <<endl;
#endif
        write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<std::to_string(i)<<"/ap_rst_n] [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_rp_reset_RST]" <<endl;
        
        j++;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/rp_acc_data] [get_bd_intf_pins acc_"<<std::to_string(i)<<"/m_axi_mem_bus]" <<endl;
        write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_data] -boundary_type upper [get_bd_intf_pins axi_interconnect_1/S0"<<std::to_string(i)<<"_AXI]" <<endl;
#ifdef FPGA_PYNQ
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_1/M00_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]" <<endl;
#elif defined(FPGA_ZCU_102) || defined(FPGA_US_96)        
        write_static_tcl << "connect_bd_intf_net -boundary_type upper [get_bd_intf_pins axi_interconnect_1/M00_AXI] [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HPC0_FPD]" <<endl;
#endif        
        write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<std::to_string(i)<<"/interrupt] [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/rp_acc_interrupt_INTERRUPT]" <<endl; 
   
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j-1)<<"/decouple_status] [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/decouple]" <<endl;
    //TODO: add ila for US 
#ifdef FPGA_PYNQ
        if(config["dart"]["partitions"][i]["debug"]){
            write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_data] [get_bd_intf_pins system_ila_"<<std::to_string(i)<<"/SLOT_0_AXI]" <<endl;
            write_static_tcl << "set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_intf_pins acc_"<<std::to_string(i)<<"/m_axi_mem_bus]" <<endl;
            write_static_tcl << "connect_bd_intf_net [get_bd_intf_pins pr_decoupler_"<<std::to_string(j-1)<<"/rp_acc_ctrl] [get_bd_intf_pins system_ila_"<<std::to_string(i)<<"/SLOT_1_AXI]" <<endl;
            write_static_tcl << "set_property HDL_ATTRIBUTE.DEBUG {true} [get_bd_intf_pins pr_decoupler_"<<std::to_string(j-1)<<"/rp_acc_ctrl]" <<endl;
            write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_interrupt_INTERRUPT] [get_bd_pins system_ila_"<<std::to_string(i)<<"/probe0]" <<endl;
            write_static_tcl << "connect_bd_net [get_bd_pins system_ila_"<<std::to_string(i)<<"/resetn] [get_bd_pins acc_"<<std::to_string(i)<<"/ap_rst_n]" <<endl;
            write_static_tcl << "connect_bd_net [get_bd_pins system_ila_"<<std::to_string(i)<<"/clk] [get_bd_pins processing_system7_0/FCLK_CLK0]" <<endl;
        }
#endif
    }
    
    //connect interconnects to master clock
    write_static_tcl << "startgroup" <<endl;
    for(i=0, j=0; i < num_partitions; i++, j+=2) {
#ifdef FPGA_PYNQ
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j+1)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/S0"<<std::to_string(i)<<"_ACLK]" <<endl;
#elif FPGA_ZCU_102 || defined(FPGA_US_96)
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/M0"<<std::to_string(j+1)<<"_ACLK]" <<endl;
        write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_1/S0"<<std::to_string(i)<<"_ACLK]" <<endl;
#endif
    }
#ifdef FPGA_PYNQ
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/ACLK]" <<endl;
    //write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M00_ACLK]" <<endl;
    //write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/M01_ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_0/S00_ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/processing_system7_0/FCLK_CLK0 (100 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}}  [get_bd_pins axi_interconnect_1/M00_ACLK]" <<endl;
#elif defined(FPGA_ZCU_102) || defined(FPGA_US_96)
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_0/S00_ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_1/ACLK]" <<endl;
    write_static_tcl << "apply_bd_automation -rule xilinx.com:bd_rule:clkrst -config { Clk {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Freq {100} Ref_Clk0 {} Ref_Clk1 {} Ref_Clk2 {}} [get_bd_pins axi_interconnect_1/M00_ACLK]" <<endl;
    //write_static_tcl <<"connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/maxihpm0_fpd_aclk] [get_bd_pins zynq_ultra_ps_e_0/pl_clk0] " <<endl;
#endif    
    write_static_tcl << "endgroup" <<endl;

    for(j=0; j < num_partitions * 2; j+=2) { 
#ifdef FPGA_PYNQ
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg_aresetn] [get_bd_pins rst_ps7_0_100M/peripheral_aresetn] " <<endl;
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/rp_rp_reset_RST] [get_bd_pins rst_ps7_0_100M/interconnect_aresetn]" <<endl;
#elif defined(FPGA_ZCU_102)  
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg_aresetn] [get_bd_pins rst_ps8_0_99M/peripheral_aresetn] " <<endl;
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/rp_rp_reset_RST] [get_bd_pins rst_ps8_0_99M/interconnect_aresetn]" <<endl;
#elif defined(FPGA_US_96) 
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg_aresetn] [get_bd_pins rst_ps8_0_100M/peripheral_aresetn] " <<endl;
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/rp_rp_reset_RST] [get_bd_pins rst_ps8_0_100M/interconnect_aresetn]" <<endl;

#endif
    }
    
    //Address map for accelerators
    //TODO: Generate the linux device tree from this mapping
    for(i=0, j=0; i < num_partitions; i++, j+=2) {
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {acc_"<<std::to_string(i)<<"/s_axi_ctrl_bus/Reg }]" <<endl;
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {pr_decoupler_"<<std::to_string(j)<<"/s_axi_reg/Reg }]" <<endl;
#ifdef FPGA_PYNQ
        write_static_tcl << "assign_bd_address [get_bd_addr_segs {processing_system7_0/S_AXI_HP0/HP0_DDR_LOWOCM }]" <<endl;
#endif
    }

    //connect interrupts
    write_static_tcl << "startgroup " <<endl;
    write_static_tcl << "create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 " <<endl;
    write_static_tcl << "endgroup " <<endl;
    write_static_tcl << "set_property -dict [list CONFIG.NUM_PORTS {"<<num_partitions<<"}] [get_bd_cells xlconcat_0]" <<endl;
    write_static_tcl << "startgroup" <<endl;
#ifdef FPGA_PYNQ
    write_static_tcl << "set_property -dict [list CONFIG.PCW_USE_FABRIC_INTERRUPT {1} CONFIG.PCW_IRQ_F2P_INTR {1}] [get_bd_cells processing_system7_0]" <<endl;
#elif defined(FPGA_ZCU_102) || defined(FPGA_US_96)
    write_static_tcl << "set_property -dict [list CONFIG.PSU__USE__IRQ0 {1}] [get_bd_cells zynq_ultra_ps_e_0] " <<endl;
#endif    
    write_static_tcl << "endgroup" <<endl;
#ifdef FPGA_PYNQ    
    write_static_tcl << "connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins processing_system7_0/IRQ_F2P]" <<endl;
#elif defined(FPGA_ZCU_102) || defined(FPGA_US_96)
    write_static_tcl << "connect_bd_net [get_bd_pins xlconcat_0/dout] [get_bd_pins zynq_ultra_ps_e_0/pl_ps_irq0]" <<endl;
#endif

    for(i=0, j=1; i < num_partitions; i++, j+=2) {
       // write_static_tcl << "connect_bd_net [get_bd_pins acc_"<<i<<"/interrupt] [get_bd_pins xlconcat_0/In"<<i<<"] " <<endl;
        write_static_tcl << "connect_bd_net [get_bd_pins pr_decoupler_"<<std::to_string(j)<<"/s_acc_interrupt_INTERRUPT] [get_bd_pins xlconcat_0/In"<<std::to_string(i)<<"]" <<endl;
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
        // if (fs::exists(fs::path(dest))){
        //     fs::remove(fs::path(dest));
        // }
        fs::copy_file(src, dest, fs::copy_options::update_existing);
    }catch (std::system_error & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not copy the DCP file" << endl;
        exit(EXIT_FAILURE);
    }
}

void pr_tool::generate_wrapper(flora *fl_ptr)
{
    int i,j,k;

    cout << "PR_TOOL: generating HDL wrappers " <<endl;

#ifndef WITH_PARTITIONING

    // for(i = 0; i < num_rm_modules; i++) {
    //     create_acc_wrapper(rm_list[i].top_module, rm_list[i].rm_tag, rm_list[i].partition_id );
    // }
    for(i = 0; i < num_rm_partitions; i++) {
        for(j = 0; j < config["dart"]["partitions"][i]["hw_ips"].size(); j++) {
            create_acc_wrapper(
                config["dart"]["partitions"][i]["hw_ips"][j]["top_name"].as<string>(),
                config["dart"]["partitions"][i]["hw_ips"][j]["ip_name"].as<string>(),
                i
            );
        }
    }

#else
    for(i = 0; i < fl_ptr->from_solver.num_partition; i++) {
        for(k = 0; k <  fl_ptr->alloc[i].num_hw_tasks_in_part; k++) {                                                      
            //create_acc_wrapper(rm_list[fl_ptr->alloc[i].task_id[k]].top_module, 
            //                    rm_list[fl_ptr->alloc[i].task_id[k]].rm_tag, 
            //                    i);
            create_acc_wrapper(
                config["dart"]["hw_ips"][fl_ptr->alloc[i].task_id[k]]["top_name"].as<std::string>(), 
                config["dart"]["hw_ips"][fl_ptr->alloc[i].task_id[k]]["ip_name"].as<std::string>(), 
                i
            );
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
