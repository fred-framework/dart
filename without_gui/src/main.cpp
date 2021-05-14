#include "flora.h"
#include "pr_tool.h"

// TODO: BIruk, I dont know if it is the usage for all configurations.
// If i's not, please extend this procedure for the other usages.
void usage(){
    cout << "pr_tool <version>, 2021, ReTiS Laboratory, Scuola Sant'Anna, Pisa, Italy\n";
    cout << "Usage:\n";
    cout << "  pr_tool <# IPs> <CSV file> <optional arguments>\n";
    cout << "     Mandatory arguments:\n";
    cout << "      * <# IPs>\n";
    cout << "      * <CSV file>\n";
    cout << "     Optional arguments:\n";
    cout << "      * --ila \n"; 
    cout << "      * --static <static top module> <static part DCP file>\n\n";
    cout << "  The current directory must be empty to receive the pr_tool project.\n\n";
    cout << "Environment variables:\n";
    cout << " - XILINX_VIVADO: points to the Vivado directory;\n";
    cout << " - DART_HOME: the source dir for DART;\n";
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


int main(int argc, char* argv[])
{
// checking the mandatory arguments
    if (!has_only_digits(argv[1])){
        cout << "ERROR: integer expected as the 1st parameter\n\n";
        usage();
        exit(1);
    }
    fs::path csv_filename(argv[2]);
    if (!fs::exists(csv_filename)){
        cout << "ERROR: CSV file '" << csv_filename.string() << "' not found\n\n";
        usage();
        exit(1);
    }
    if (!fs::is_empty(fs::current_path())){
        cout << "ERROR: the current directory '" << fs::current_path().string() << "' must be empty.\n\n";
        exit(1);
    }

    // checking the optional arguments
    string static_top_module = "";
    string static_dcp_file = "";
    bool use_ila=false;
    // the 1st 3 arguments are mandatory. start searching in the 4th argument
    for (int i = 3; i < argc; ++i) {
        // in case the static part is defined by the used
        if (std::string(argv[i]) == "--static") {
            if (i + 2 < argc) { // Make sure we aren't at the end of argv!
                // Increment 'i' so we don't get the argument as the next argv[i].
                static_top_module = argv[i++];
                static_dcp_file = argv[i++];
                // testing the static part DCP file
                if (!fs::exists(static_dcp_file)){
                    cout << "ERROR: DCP file '" << static_dcp_file << "' not found\n\n";
                    usage();
                    exit(1);
                }
                if (fs::path(static_dcp_file).extension() != ".dcp"){
                    cout << "ERROR: expecting DCP file extension but got '" << static_dcp_file << "'\n\n";
                    usage();
                    exit(1);
                }

            } else {
                std::cerr << "--static option requires two arguments: the static top name and the static dcp file" << std::endl;
                usage();
                exit(1);
            }  
        }
        if (std::string(argv[i]) == "--ila") {
            use_ila = true;
        }
    }

// checking DART_HOME enrironment variables
    string dart_path = getEnvVar ("DART_HOME");
    if (dart_path.empty()){
        cout << "ERROR: DART_HOME environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(dart_path) || !fs::is_directory(dart_path)){
        cout << "ERROR: DART_HOME points to an invalid directory '" << dart_path << "'\n";
        exit(1);
    }  

// checking DART_HOME enrironment variables
    string dart_ip_path = getEnvVar ("DART_IP_PATH");
    if (dart_ip_path.empty()){
        cout << "ERROR: DART_IP_PATH environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(dart_ip_path) || !fs::is_directory(dart_ip_path)){
        cout << "ERROR: DART_IP_PATH points to an invalid directory '" << dart_ip_path << "'\n";
        exit(1);
    } 

// checking GUROBI_HOME enrironment variables
    string gurobi_path = getEnvVar ("GUROBI_HOME");
    if (gurobi_path.empty()){
        cout << "ERROR: GUROBI_HOME environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(gurobi_path) || !fs::is_directory(gurobi_path)){
        cout << "ERROR: GUROBI_HOME points to an invalid directory '" << gurobi_path << "'\n";
        exit(1);
    }  
    gurobi_path = getEnvVar ("GRB_LICENSE_FILE");
    if (gurobi_path.empty()){
        cout << "ERROR: GRB_LICENSE_FILE environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(gurobi_path) || !fs::is_regular_file(gurobi_path)){
        cout << "ERROR: GRB_LICENSE_FILE points to an invalid file '" << gurobi_path << "'\n";
        exit(1);
    }  

// check vivado enrironment variable used by tools/start_vivado    
    string vivado_path = getEnvVar ("XILINX_VIVADO");
    if (vivado_path.empty()){
        cout << "ERROR: XILINX_VIVADO environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(vivado_path) || !fs::is_directory(vivado_path)){
        cout << "ERROR: XILINX_VIVADO points to an invalid file '" << vivado_path << "'\n";
        exit(1);
    } 

// check vivado is in the path and its version
    try{
        FILE *vivado_out_p = popen("vivado -version", "r");

        if (!vivado_out_p)
        {
            cout << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(1);
        }

        char buffer[1024];
        // get only the 1st line to check the vivado version
        fgets(buffer, sizeof(buffer), vivado_out_p);
        pclose(vivado_out_p);
        string line_with_vivado_version = buffer;
        if (line_with_vivado_version.find("Vivado") ==std::string::npos){
            cout << "ERROR: vivado not found. please set it in the PATH environment variable\n";
            exit(1);        
        }
        vector<string> tokens = split(line_with_vivado_version, ' ');
        if (! (tokens[1] == "v2019.2" || tokens[1] == "v2018.3")){
            cout << "WARNING: expecting vivado version 'v2018.3' or 'v2019.2' but found '" << tokens[1] << "'\n";
            cout << "unexpected errors might occur with different Vivado versions\n";
        }
    }
    catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    } 

// start doing usefull stuff ...
#ifdef RUN_FLORA
    input_to_flora in_flora;
    // the mandatory arguments for flora
    in_flora.num_rm_modules = atol(argv[1]);
    //in_flora.type_of_fpga = (fpga_type) atol(argv[2]);
    in_flora.path_to_input = argv[2];
    // set the optinal arguments
    // TODO: that's BAD. attributes in input_to_flora and input_to_pr are duplicated 
    in_flora.use_ila = use_ila;

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
        pr_input.path_to_input = argv[2];

        // set the optional arguments for pr
        pr_input.use_ila = use_ila;
        pr_input.static_top_module = static_top_module;
        pr_input.static_dcp_file = static_dcp_file;

        // where the project will be created
        pr_input.path_to_output = fs::current_path().string();

        pr_tool tool(&pr_input);
#endif    
    return 0;

}
