#include <algorithm>
#include <yaml-cpp/yaml.h>

#include "flora.h"
#include "pr_tool.h"
#include "version.h"

// TODO: BIruk, I dont know if it is the usage for all configurations.
// If i's not, please extend this procedure for the other usages.
void usage(){
    cout << "pr_tool <version>, 2021, ReTiS Laboratory, Scuola Sant'Anna, Pisa, Italy\n";
    cout << "Usage:\n";
    #ifdef WITH_PARTITIONING
    cout << "  run_pr_tool_with_part_* <# IPs> <CSV file> <optional arguments>\n";
    cout << "     Mandatory arguments:\n";
    cout << "      * <# IPs>\n";
    #else
    cout << "  run_pr_tool_without_part_* <# reconf_regions> <CSV file> <optional arguments>\n";
    cout << "     Mandatory arguments:\n";
    cout << "      * <# reconf_regions>\n";
    #endif
    cout << "      * <CSV file>\n";
    cout << "     Optional arguments:\n";
    cout << "      * --ila \n"; 
    cout << "      * --static <static top module> <static part DCP file>\n\n";
    cout << "  The current directory must be empty to receive the pr_tool project.\n\n";
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

// lets make the configuration file global, against all recommendations
YAML::Node config;

int main(int argc, char* argv[])
{
    //YAML::Node config = YAML::LoadFile("config.yaml");

    vector<string> tokens ;
    if (argc <2){
        cerr << "ERROR: mandatory arguments are missing\n\n";
        usage();
        exit(1);
    }
// checking the mandatory arguments
/*
    if (!has_only_digits(argv[1])){
        cerr << "ERROR: integer expected as the 1st parameter\n\n";
        usage();
        exit(1);
    }
    */
    fs::path csv_filename(argv[1]);
    if (!fs::exists(csv_filename)){
        cerr << "ERROR: CSV file '" << csv_filename.string() << "' not found\n\n";
        usage();
        exit(1);
    }
    /*
    if (!fs::is_empty(fs::current_path())){
        cerr << "ERROR: the current directory '" << fs::current_path().string() << "' must be empty.\n\n";
        exit(1);
    }
*/
    config = YAML::LoadFile(csv_filename.string().c_str());
    /*
    std::cout << "Parsed YAML:\n" << config << "\n";
    std::cout << "Parsed YAML:\n" << config["flora"] << "\n";
    std::cout << "Parsed YAML:\n" << config["flora"][0] << "\n";
    std::cout << "1o IP da part 0: " << config["flora"][0]["ips_list"][0] << "\n";
    std::cout << "Parsed YAML:\n" << config["flora"][1] << "\n";
    */
    // checking the optional arguments
    //string static_top_module = "";
    //string static_dcp_file = "";
/*
    // the 1st 3 arguments are mandatory. start searching in the 4th argument
    for (int i = 3; i < argc; ++i) {
        string arg = argv[i];
        // remove white spaces. this happens when dart is called from the python flow
        arg.erase(std::remove(arg.begin(), arg.end(), ' '), arg.end());
        // in case the static part is defined by the used
        if (arg == "--static") {
            if (i + 2 < argc) { // Make sure we aren't at the end of argv!
                // Increment 'i' so we don't get the argument as the next argv[i].
                static_top_module = argv[i++];
                static_dcp_file = argv[i++];
                // testing the static part DCP file
                if (!fs::exists(static_dcp_file)){
                    cerr << "ERROR: DCP file '" << static_dcp_file << "' not found\n\n";
                    usage();
                    exit(1);
                }
                if (fs::path(static_dcp_file).extension() != ".dcp"){
                    cerr << "ERROR: expecting DCP file extension but got '" << static_dcp_file << "'\n\n";
                    usage();
                    exit(1);
                }
                cout << "Using static part in file " << static_dcp_file << " with name " << static_top_module <<endl;
            } else {
                cerr << "ERROR: --static option requires two arguments: the static top name and the static dcp file" << std::endl;
                usage();
                exit(1);
            }  
        }else if (arg == "--ila") {
            use_ila = true;
            cout << "Using ILA for hardware debuging" <<endl;
        } else {
            cerr << "ERROR: unrecognized argument " << argv[i] << std::endl;
            usage();
            exit(1);
        }
    }
*/
// checking DART_HOME enrironment variables
    string dart_path = getEnvVar ("DART_HOME");
    if (dart_path.empty()){
        cerr << "ERROR: DART_HOME environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(dart_path) || !fs::is_directory(dart_path)){
        cerr << "ERROR: DART_HOME points to an invalid directory '" << dart_path << "'\n";
        exit(1);
    }  

// checking DART_HOME enrironment variables
    string dart_ip_path = getEnvVar ("DART_IP_PATH");
    if (dart_ip_path.empty()){
        cerr << "ERROR: DART_IP_PATH environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(dart_ip_path) || !fs::is_directory(dart_ip_path)){
        cerr << "ERROR: DART_IP_PATH points to an invalid directory '" << dart_ip_path << "'\n";
        exit(1);
    } 

// checking GUROBI_HOME enrironment variables
    string gurobi_path = getEnvVar ("GUROBI_HOME");
    if (gurobi_path.empty()){
        cerr << "ERROR: GUROBI_HOME environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(gurobi_path) || !fs::is_directory(gurobi_path)){
        cerr << "ERROR: GUROBI_HOME points to an invalid directory '" << gurobi_path << "'\n";
        exit(1);
    }  
    gurobi_path = getEnvVar ("GRB_LICENSE_FILE");
    if (gurobi_path.empty()){
        cerr << "ERROR: GRB_LICENSE_FILE environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(gurobi_path) || !fs::is_regular_file(gurobi_path)){
        cerr << "ERROR: GRB_LICENSE_FILE points to an invalid file '" << gurobi_path << "'\n";
        exit(1);
    }  
    int GRB_major,GRB_minor,GRB_tech;
    // DART was tested with gurobi 911 but it didnt work. So far, it works only with version 811
    GRBversion(&GRB_major,&GRB_minor,&GRB_tech);
    if (GRB_major != 8 || GRB_minor != 1 || GRB_tech != 1){
        cerr << "ERROR: The expected gurobi version is 811, but found '" << GRB_major << GRB_minor << GRB_tech << "'\n";
        exit(1);
    }

// check vivado enrironment variable used by tools/start_vivado    
    string vivado_path = getEnvVar ("XILINX_VIVADO");
    Version vivado_version;
    if (vivado_path.empty()){
        cerr << "ERROR: XILINX_VIVADO environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(vivado_path) || !fs::is_directory(vivado_path)){
        cerr << "ERROR: XILINX_VIVADO points to an invalid file '" << vivado_path << "'\n";
        exit(1);
    } 

// check vivado is in the path and its version
    try{
        FILE *vivado_out_p = popen("vivado -version", "r");

        if (!vivado_out_p)
        {
            cerr << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(1);
        }

        char buffer[1024];
        // get only the 1st line to check the vivado version
        fgets(buffer, sizeof(buffer), vivado_out_p);
        pclose(vivado_out_p);
        string line_with_vivado_version = buffer;
        if (line_with_vivado_version.find("Vivado") ==std::string::npos){
            cerr << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(1);        
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
        cout << "YAML \n" << config << "\n"; 
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
    input_to_pr pr_input;
    #ifdef WITH_PARTITIONING    
        pr_input.num_rm_modules = atol(argv[1]); 
    #else
        pr_input.num_rm_partitions = atol(argv[1]); 
    #endif
        //pr_input.type_of_fpga = (fpga_type) atol(argv[2]);
        //pr_input.path_to_input = argv[2];

        // set the optional arguments for pr
        //pr_input.use_ila = use_ila;
        //pr_input.static_top_module = static_top_module;
        //pr_input.static_dcp_file = static_dcp_file;

        // where the project will be created
        pr_input.path_to_output = fs::current_path().string();
        /*
        Version min_version2("2020.1.0.0");
        if (vivado_version > min_version2){
        //if (tokens[1] == "v2020.2" || tokens[1] == "v2020.1" || tokens[1] == "v2020.3" || tokens[1] == "v2021.1" || tokens[1] == "v2021.2")
            pr_input.vivado_version = 0;
        }else{
            pr_input.vivado_version = 1;
        }
        */

        pr_tool tool(&pr_input);
#endif    
    return 0;

}
