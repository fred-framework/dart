#include <algorithm>
#include <regex>
#include <yaml-cpp/yaml.h>

#include "flora.h"
#include "pr_tool.h"
#include "version.h"

// lets make the configuration file global, against all recommendations
YAML::Node config;

void usage(){
    cout << "Dart, rev "<< GIT_REV << ", 2021, ReTiS Laboratory, Scuola Sant'Anna, Pisa, Italy\n";
    cout << "Last commit time: " << GIT_DATE <<endl;
    #ifdef WITH_PARTITIONING
    cout << "  - Partitioning mode ON;" <<endl;
    #else
    cout << "  - Partitioning mode OFF;" <<endl;
    #endif
    #ifdef FPGA_PYNQ
    cout << "  - Pynq board;" <<endl;
    cout << "  - FPGA device: xc7z020clg400-1" <<endl;
    #elif FPGA_ZYNQ
    cout << "  - Zynq board;" <<endl;
    cout << "  - FPGA device: xc7z010clg400-1" <<endl;
    #elif FPGA_ZCU_102
    cout << "  - Zynq board;" <<endl;
    cout << "  - FPGA device: xczu9eg-ffvb1156-2-e" <<endl;
    #elif FPGA_US_96
    cout << "  - Ultra96-V2 board;" <<endl;
    cout << "  - FPGA device: xczu3eg-sbva484-1-e" <<endl;
    #else
        #error "FPGA not suported!"
    #endif    
    cout << "Usage:\n";
    cout << "  dart <YAML file>\n";
    cout << "Environment variables:\n";
    cout << " - XILINX_VIVADO: points to the Vivado directory;\n";
    cout << " - DART_HOME: the source dir for DART. It must be gurobi version 8.1.1;\n";
    cout << " - DART_IP_PATH: the directory where the IPs are stored;\n";
    cout << " - GUROBI_HOME: the home dir for gurobi installation;\n";
    cout << " - GRB_LICENSE_FILE: Gurobi's license file.\n\n";
}

bool has_only_digits(const string s){
  return s.find_first_not_of( "0123456789" ) == string::npos;
}

std::string getEnvVar( std::string const & key )
{
    char * val = getenv( key.c_str() );
    return val == NULL ? std::string("") : std::string(val);
}

vector<string> split(const string& text, char delimiter) {
    string tmp;
    vector<string> stk;
    stringstream ss(text);
    while(getline(ss,tmp, delimiter)) {
        stk.push_back(tmp);
    }
    return stk;
}

// check whether the environment are corretly set up
void checking_requirements(void){
// checking DART_HOME enrironment variables
    string dart_path = getEnvVar ("DART_HOME");
    vector<string> tokens ;

    if (dart_path.empty()){
        cerr << "ERROR: DART_HOME environment variable is not defined\n\n";
        exit(EXIT_FAILURE);
    }
    if (!fs::exists(dart_path) || !fs::is_directory(dart_path)){
        cerr << "ERROR: DART_HOME points to an invalid directory '" << dart_path << "'\n";
        exit(EXIT_FAILURE);
    }  

// checking DART_HOME enrironment variables
    string dart_ip_path = getEnvVar ("DART_IP_PATH");
    if (dart_ip_path.empty()){
        cerr << "ERROR: DART_IP_PATH environment variable is not defined\n\n";
        exit(EXIT_FAILURE);
    }
    if (!fs::exists(dart_ip_path) || !fs::is_directory(dart_ip_path)){
        cerr << "ERROR: DART_IP_PATH points to an invalid directory '" << dart_ip_path << "'\n";
        exit(EXIT_FAILURE);
    }

// checking GUROBI_HOME enrironment variables
    string gurobi_path = getEnvVar ("GUROBI_HOME");
    if (gurobi_path.empty()){
        cerr << "ERROR: GUROBI_HOME environment variable is not defined\n\n";
        exit(EXIT_FAILURE);
    }
    if (!fs::exists(gurobi_path) || !fs::is_directory(gurobi_path)){
        cerr << "ERROR: GUROBI_HOME points to an invalid directory '" << gurobi_path << "'\n";
        exit(EXIT_FAILURE);
    }  
    gurobi_path = getEnvVar ("GRB_LICENSE_FILE");
    if (gurobi_path.empty()){
        cerr << "ERROR: GRB_LICENSE_FILE environment variable is not defined\n\n";
        exit(EXIT_FAILURE);
    }
    if (!fs::exists(gurobi_path) || !fs::is_regular_file(gurobi_path)){
        cerr << "ERROR: GRB_LICENSE_FILE points to an invalid file '" << gurobi_path << "'\n";
        exit(EXIT_FAILURE);
    }  
    int GRB_major,GRB_minor,GRB_tech;
    // DART was tested with gurobi 911 but it didnt work. So far, it works only with version 811
    GRBversion(&GRB_major,&GRB_minor,&GRB_tech);
    if (GRB_major != 8 || GRB_minor != 1 || GRB_tech != 1){
        cerr << "ERROR: The expected gurobi version is 811, but found '" << GRB_major << GRB_minor << GRB_tech << "'\n";
        exit(EXIT_FAILURE);
    }

// check vivado enrironment variable used by tools/start_vivado    
    string vivado_path = getEnvVar ("XILINX_VIVADO");
    Version vivado_version;
    if (vivado_path.empty()){
        cerr << "ERROR: XILINX_VIVADO environment variable is not defined\n\n";
        exit(EXIT_FAILURE);
    }
    if (!fs::exists(vivado_path) || !fs::is_directory(vivado_path)){
        cerr << "ERROR: XILINX_VIVADO points to an invalid file '" << vivado_path << "'\n";
        exit(EXIT_FAILURE);
    } 

// check vivado is in the path and its version
    try{
        FILE *vivado_out_p = popen("vivado -version", "r");

        if (!vivado_out_p)
        {
            cerr << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(EXIT_FAILURE);
        }

        char buffer[1024];
        // get only the 1st line to check the vivado version
        fgets(buffer, sizeof(buffer), vivado_out_p);
        pclose(vivado_out_p);
        string line_with_vivado_version = buffer;
        if (line_with_vivado_version.find("Vivado") ==std::string::npos){
            cerr << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(EXIT_FAILURE);        
        }
        tokens = split(line_with_vivado_version, ' ');
        string vivado_version_str;
        unsigned vivado_major_version,vivado_minor_version;
        vivado_version_str = tokens[1];
        // erase the initial v in vivado version name
        vivado_version_str.erase(0, 1);
        // including revision and build numbers to make it compatible with the Version class
        vivado_version_str += ".0.0";
        // assing the YAML configuration
        vivado_major_version = atoi(split(vivado_version_str, '.')[0].c_str());
        vivado_minor_version = atoi(split(vivado_version_str, '.')[1].c_str());
        config["vivado_version_str"] = vivado_version_str;
        config["vivado_major_version"] = vivado_major_version;
        config["vivado_minor_version"] = vivado_minor_version;
        //cout << "YAML \n" << config << "\n"; 
        // compare vivado versions
        vivado_version = vivado_version_str;
        Version min_version("2018.3.0.0");
        if (vivado_version < min_version){
            cerr << "WARNING: expecting vivado version '2018.3' or newer but found '" << vivado_version << "'\n";
            cerr << "unexpected errors might occur with different Vivado versions\n";
        }
        /*
        if (! (tokens[1] == "v2019.2" || tokens[1] == "v2018.3")){
            cerr << "WARNING: expecting vivado version 'v2018.3' or 'v2019.2' but found '" << tokens[1] << "'\n";
            cerr << "unexpected errors might occur with different Vivado versions\n";
        }
        */
    }
    catch (std::system_error & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: could not check vivado version\n";
        exit(EXIT_FAILURE);
    } 
}

void check_string_key(YAML::Node key,string key_name){
    string value;
    const string name_regex_str("[a-zA-Z][a-zA-Z0-9_]*");
    regex str_regex(name_regex_str);

    try{
        value = key.as<string>();
    } catch (const std::exception &e) {
        cerr << "ERROR: key '" << key_name <<"' expects string format" << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key '" << key_name <<"' expects string format2" << endl;
        exit(EXIT_FAILURE);
    }
    // check the expected name format
    if (!regex_match(value,str_regex)){
        cerr << "ERROR: key '" << key_name <<"' expects format " << name_regex_str << endl;
        exit(EXIT_FAILURE);
    }
}

void check_ip_keys(YAML::Node ip_node,string key_msg){
    int k, buff_size;
    // check name
    if (!ip_node["ip_name"]){
        cerr << "ERROR: key 'ip_name' not found in " << key_msg << endl;
        exit(EXIT_FAILURE);
    }
    check_string_key(ip_node["ip_name"],"ip_name");
    // check top_name
    if (!ip_node["top_name"]){
        cerr << "ERROR: key 'top_name' not found in " << key_msg << endl;
        exit(EXIT_FAILURE);
    }
    check_string_key(ip_node["top_name"],"top_name");

    // it is mandatory to have timeout both with partition ON or OFF
    if (!ip_node["timeout"]){
        cerr << "ERROR: key 'timeout' not found in " << key_msg << endl;
        exit(EXIT_FAILURE);
    }
    unsigned int aux_uint;
    try{
        // all values must be unsigned int
        aux_uint = ip_node["timeout"].as<unsigned int>();
    } catch (const std::exception &e) {
        cerr << "ERROR: key 'timeout' expects unsigned int values. See " << key_msg << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key 'timeout' expects unsigned int values. See " << key_msg << endl;
        exit(EXIT_FAILURE);
    }

    // it is mandatory to have at least two buffers; one for data in and the other to data out
    if (!ip_node["buffers"]){
        cerr << "ERROR: key 'buffers' not found in " << key_msg << endl;
        exit(EXIT_FAILURE);
    }
    try{
        if (ip_node["buffers"].size()<2){
            cerr << "ERROR: key 'buffers' expects at least two values. See " << key_msg << endl;
            exit(EXIT_FAILURE);
        }
        // all buffer size values must be unsigned int
        for (k=0;k<ip_node["buffers"].size();k++){
            buff_size = ip_node["buffers"][k].as<unsigned int>();
        }
    } catch (const std::exception &e) {
        // std::cerr << __FUNCTION__ << " caught unhandled exception. what(): "
        //         << e.what() << std::endl;
        cerr << "ERROR: key 'buffers' expects unsigned int values. See " << key_msg << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key 'buffers' expects unsigned int values. See " << key_msg << endl;
        exit(EXIT_FAILURE);
    }
}

// check whether the yaml has the expected format
// TODO: in the future we could use a YAML schema validator, like this one https://github.com/psranga/yavl-cpp
void check_yaml(void){
    int i,j;
    unsigned int aux_uint;
    string ip_name, top_name, key_name, node_msg;

    if (!config["dart"]){
        cerr << "ERROR: the YAML does not comply with DART format\n\n";
        exit(EXIT_FAILURE);
    }
    // checking for unsupported keys under the root
    for(YAML::const_iterator it=config.begin();it!=config.end();++it) {
        key_name = it->first.as<std::string>();
        if (key_name != "dart" && key_name != "skip_ip_synthesis" && key_name != "skip_static_synthesis"){
            cerr << "ERROR: unexpected key '" << key_name << "' under the YAML root" << endl;
            exit(EXIT_FAILURE);
        }
    }
    // testing the boolean flags
    bool test;
    try{
        if (config["skip_ip_synthesis"]){
            test = config["skip_ip_synthesis"].as<bool>();
        }
    } catch (const std::exception &e) {
        cerr << "ERROR: key 'skip_ip_synthesis' must be True or False" << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key 'skip_ip_synthesis' must be True or False" << endl;
        exit(EXIT_FAILURE);
    }
    try{
        if (config["skip_static_synthesis"]){
            test = config["skip_static_synthesis"].as<bool>();
        }
    } catch (const std::exception &e) {
        cerr << "ERROR: key 'skip_static_synthesis' must be True or False" << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key 'skip_static_synthesis' must be True or False" << endl;
        exit(EXIT_FAILURE);
    }
#ifdef WITH_PARTITIONING
    // a list IPs is mandatory
    if (!config["dart"]["hw_ips"]){
        cerr << "ERROR: key 'hw_ips' not found in the YAML file\n\n";
        exit(EXIT_FAILURE);
    }
    if (!config["dart"]["num_partitions"]){
        cerr << "ERROR: key 'num_partitions' not found in the YAML file\n\n";
        exit(EXIT_FAILURE);
    }
    try{
        // must be unsigned int > 0
        aux_uint = config["dart"]["num_partitions"].as<unsigned int>();
        if (aux_uint < 1){
            cerr << "ERROR: key 'num_partitions' expects unsigned int values greater than 0." << endl;
            exit(EXIT_FAILURE);
        }
    } catch (const std::exception &e) {
        cerr << "ERROR: key 'num_partitions' expects unsigned int values greater than 0." << endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        cerr << "ERROR: key 'num_partitions' expects unsigned int values greater than 0. "  << endl;
        exit(EXIT_FAILURE);
    }

    // checking for unsupported keys under dart
    for(YAML::const_iterator it=config["dart"].begin();it!=config["dart"].end();++it) {
        key_name = it->first.as<std::string>();
        if (key_name != "hw_ips" && key_name != "num_partitions" &&
            key_name != "static_top_module" && key_name != "static_dcp_file")
        {
            cerr << "ERROR: unexpected key '" << key_name << "' under 'dart'" << endl;
            exit(EXIT_FAILURE);
        }

    }
    if (config["dart"]["hw_ips"].size() < 1){
        cerr << "ERROR: at least one IP is requried" << endl;
        exit(EXIT_FAILURE);
    }
    for (i=0;i<config["dart"]["hw_ips"].size();i++){
        YAML::Node ip_node = config["dart"]["hw_ips"][i];
        node_msg = "IP '" + std::to_string(i) + "'";
        // checking for unsupported keys for an IP
        for(YAML::const_iterator it=ip_node.begin();it!=ip_node.end();++it) {
            key_name = it->first.as<std::string>();
            if (key_name != "ip_name" && key_name != "top_name" && key_name != "buffers" &&
                key_name != "slack_time" && key_name != "wcet" && key_name != "timeout")
            {
                cerr << "ERROR: unexpected key '" << key_name << "' in " << node_msg << endl;
                exit(EXIT_FAILURE);
            }
        }
        // check name, top_module, and buffers keys
        check_ip_keys(ip_node,node_msg);
        // check wcet and slack_time keys
        if (!ip_node["wcet"]){
            cerr << "ERROR: key 'wcet' not found in " << node_msg << endl;
            exit(EXIT_FAILURE);
        }
        if (!ip_node["slack_time"]){
            cerr << "ERROR: key 'slack_time' not found in " << node_msg << endl;
            exit(EXIT_FAILURE);
        }
        unsigned int aux_uint;
        try{
            // all values must be unsigned int
            aux_uint = ip_node["wcet"].as<unsigned int>();
        } catch (const std::exception &e) {
            cerr << "ERROR: key 'wcet' expects unsigned int values. See " << node_msg << endl;
            exit(EXIT_FAILURE);
        } catch (...) {
            cerr << "ERROR: key 'wcet' expects unsigned int values. See " << node_msg << endl;
            exit(EXIT_FAILURE);
        }
        try{
            // all values must be unsigned int
            aux_uint = ip_node["slack_time"].as<unsigned int>();
        } catch (const std::exception &e) {
            cerr << "ERROR: key 'slack_time' expects unsigned int values. See " << node_msg << endl;
            exit(EXIT_FAILURE);
        } catch (...) {
            cerr << "ERROR: key 'slack_time' expects unsigned int values. See " << node_msg << endl;
            exit(EXIT_FAILURE);
        }          
    }
#else
    // a partition list is mandatory
    if (!config["dart"]["partitions"]){
        cerr << "ERROR: key 'partitions' not found in the YAML file\n\n";
        exit(EXIT_FAILURE);
    }
    // checking for unsupported keys under dart
    for(YAML::const_iterator it=config["dart"].begin();it!=config["dart"].end();++it) {
        key_name = it->first.as<std::string>();
        if (key_name != "partitions" && key_name != "static_top_module" && key_name != "static_dcp_file"){
            cerr << "ERROR: unexpected key '" << key_name << "' under 'dart'" << endl;
            exit(EXIT_FAILURE);
        }
    }
    // all partitions must have the 'hw_ips' key
    for (i=0;i<config["dart"]["partitions"].size();i++){
        if (!config["dart"]["partitions"][i]["hw_ips"]){
            cerr << "ERROR: key 'hw_ips' not found in partition '" << i << "' in the YAML file\n\n";
            exit(EXIT_FAILURE);
        }
    }
    // check the mandatory attributes of each IP
    for (i=0;i<config["dart"]["partitions"].size();i++){
        for (j=0;j<config["dart"]["partitions"][i]["hw_ips"].size();j++){
            YAML::Node part_node = config["dart"]["partitions"][i];
            // checking for unsupported keys for a partition
            for(YAML::const_iterator it=part_node.begin();it!=part_node.end();++it) {
                key_name = it->first.as<std::string>();
                if (key_name != "hw_ips" && key_name != "debug"){
                    cerr << "ERROR: unexpected key '" << key_name << "' in partition '" << i<< endl;
                    exit(EXIT_FAILURE);
                }
            }
            // checking the debug mode
            if (part_node["debug"]){
                // debug has the optional key data_depth 
                for(YAML::const_iterator it=part_node["debug"].begin();it!=part_node["debug"].end();++it) {
                    key_name = it->first.as<std::string>();
                    if (key_name != "data_depth" ){
                        cerr << "ERROR: unexpected key '" << key_name << "' in debug for partition '" << i<< endl;
                        exit(EXIT_FAILURE);
                    }
                }
                // data_depth must be integer and power of 2
                unsigned int ila_buffer_depth;
                try{
                    // all values must be unsigned int
                    ila_buffer_depth = config["dart"]["partitions"][i]["debug"]["data_depth"].as<unsigned int>();
                } catch (const std::exception &e) {
                    cerr << "ERROR: key 'data_depth' expects unsigned int, power of 2 values between 1024 and 131072." << endl;
                    exit(EXIT_FAILURE);
                } catch (...) {
                    cerr << "ERROR: key 'data_depth' expects unsigned int, power of 2 values between 1024 and 131072." << endl;
                    exit(EXIT_FAILURE);
                }                
                // check if ila_buffer_depth is power of 2
                if (ceil(log2(ila_buffer_depth)) != floor(log2(ila_buffer_depth)))
                { 
                    cerr<<"ERROR: ILA buffer depth must be power of 2. Found "<<  ila_buffer_depth <<endl; 
                    exit(EXIT_FAILURE);               
                }
                // WARNING: not sure if these limits are true for every device. They are true for Pynq
                if (ila_buffer_depth < 1024)
                { 
                    cerr<<"ERROR: ILA buffer depth must be at least 1024. Found "<<  ila_buffer_depth <<endl;
                    exit(EXIT_FAILURE);                
                }
                if (ila_buffer_depth > 131072)
                { 
                    cerr<<"ERROR: ILA buffer depth must be at most 131072. Found "<<  ila_buffer_depth <<endl;
                    exit(EXIT_FAILURE);                
                }
            }

            // checking the ip
            YAML::Node ip_node = config["dart"]["partitions"][i]["hw_ips"][j];
            node_msg = "IP in '" + std::to_string(j) + "', partition '" + std::to_string(i) + "'";
            // checking for unsupported keys for an IP
            for(YAML::const_iterator it=ip_node.begin();it!=ip_node.end();++it) {
                key_name = it->first.as<std::string>();
                if (key_name != "ip_name" && key_name != "top_name" && key_name != "buffers" && key_name != "timeout"){
                    cerr << "ERROR: unexpected key '" << key_name << "' in " << node_msg << endl;
                    exit(EXIT_FAILURE);
                }
            }
            // check name, top_module, and buffers keys
            check_ip_keys(ip_node,node_msg);
        }

    }
#endif
}

int main(int argc, char* argv[])
{
    
    if (argc <2){
        cerr << "ERROR: mandatory arguments are missing\n\n";
        usage();
        exit(EXIT_FAILURE);
    }

    // input file checking and parsing
    fs::path yaml_filename(argv[1]);
    if (!fs::exists(yaml_filename)){
        cerr << "ERROR: YAML file '" << yaml_filename.string() << "' not found\n\n";
        usage();
        exit(EXIT_FAILURE);
    }
    try{
        config = YAML::LoadFile(yaml_filename.string().c_str());
    }
    catch (YAML::ParserException & e)
    {
        cerr << "Exception :: " << e.what() << endl;
        cerr << "ERROR: Syntax error in the YAML " << yaml_filename << endl;
        exit(EXIT_FAILURE);
    }
    // check whether the yaml has the expected format
    check_yaml();

    // checking the required enrivonment variables
    checking_requirements();

    // print the executed command
    cout << "$> ";
    for (int i=0;i<argc;i++){
        cout << argv[i] << " ";
    }
    cout << "\n";

// start doing usefull stuff ...
#ifdef RUN_FLORA
    input_to_flora in_flora;
    // the mandatory arguments for flora
    in_flora.num_rm_modules = atol(argv[1]);
    //in_flora.type_of_fpga = (fpga_type) atol(argv[2]);
    //in_flora.path_to_input = argv[2];
    // set the optinal arguments
    // TODO: that's BAD. attributes in input_to_flora and input_to_pr are duplicated 
    //in_flora.use_ila = use_ila;

    flora fl(&in_flora);
    fl.clear_vectors();
    fl.prep_input();
    fl.start_optimizer();
//    fl.generate_xdc();
#else
        pr_tool tool(fs::current_path().string());
#endif    
    return 0;

}
