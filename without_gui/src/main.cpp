#include "flora.h"
#include "pr_tool.h"

// TODO: BIruk, I dont know if it is the usage for all configurations.
// If i's not, please extend this procedure for the other usages.
void usage(){
    cout << "pr_tool <version>, 2020, ReTiS Laboratory, Scuola Sant'Anna, Pisa, Italy\n";
    cout << "Usage:\n";
    cout << "  pr_tool <# IPs> <CSV file>\n";
    cout << "  the current directory must be empty to receive the pr_tool project.\n\n";
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

// checking arguments
    if (argc != 3){
        cout << "ERROR: invalid usage\n\n";
        usage();
        exit(1);
    }else{
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

    }
// checking enrironment variables
    string dart_path = getEnvVar ("DART_HOME");
    if (dart_path.empty()){
        cout << "ERROR: DART_HOME environment variable is not defined\n\n";
        exit(1);
    }
    if (!fs::exists(dart_path) || !fs::is_directory(dart_path)){
        cout << "ERROR: DART_HOME points to an invalid directory '" << dart_path << "'\n";
        exit(1);
    }  

// check vivado is in the path and its version
// TODO
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
        if (tokens[1] != "v2018.3"){
            cout << "ERROR: expecting vivado version 'v2018.3' but found '" << tokens[1] << "'\n";
            exit(1);        
        }
    }
    catch (std::system_error & e)
    {
        std::cerr << "Exception :: " << e.what();
    } 

// start doing usefull stuff ...
#ifdef RUN_FLORA
    input_to_flora in_flora;
    in_flora.num_rm_modules = atol(argv[1]);
    //in_flora.type_of_fpga = (fpga_type) atol(argv[2]);
    in_flora.path_to_input = argv[2];

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
    // where the project will be created
    pr_input.path_to_output = fs::current_path().string();

    pr_tool tool(&pr_input);
#endif    
    return 0;

}
