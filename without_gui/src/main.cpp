#include "flora.h"
#include "pr_tool.h"

// TODO: BIruk, I dont know if it is the usage for all configurations.
// If i's not, please extend this procedure for the other usages.
void usage(){
    cout << "pr_tool <version>, 2020, ReTiS Laboratory, Scuola Sant'Anna, Pisa, Italy\n";
    cout << "Usage:\n";
    cout << "  pr_tool <# IPs> <CSV file> <project_dir>\n\n";
}

bool has_only_digits(const string s){
  return s.find_first_not_of( "0123456789" ) == string::npos;
}

std::string getEnvVar( std::string const & key )
{
    char * val = getenv( key.c_str() );
    return val == NULL ? std::string("") : std::string(val);
}

int main(int argc, char* argv[])
{

// checking arguments
    if (argc != 4){
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
        fs::path output_path(argv[3]);
        if (fs::exists(output_path)){
            cout << "ERROR: the output project directory must not exist\n\n";
            exit(1);
        }
        if (!fs::create_directory(output_path)){
            cout << "ERROR: the output project directory cannot be created. Check the write access to this path.\n\n";
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
    pr_input.path_to_output = argv[3];

    pr_tool tool(&pr_input);
#endif    
    return 0;

}
