#include "flora.h"
#include "pr_tool.h"

int main(int argc, char* argv[])
{

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

    pr_tool tool(&pr_input);
#endif    
    return 0;

}
