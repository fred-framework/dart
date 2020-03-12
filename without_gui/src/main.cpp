#include "flora.h"

int main(int argc, char* argv[])
{

    input_to_flora in_flora;
    in_flora.num_rm_modules = atol(argv[1]);
    in_flora.type_of_fpga = (fpga_type) atol(argv[2]);
    in_flora.path_to_input = argv[3];

    flora fl(&in_flora);
    fl.clear_vectors();
    fl.prep_input();
    fl.start_optimizer();
    fl.generate_xdc();

    return 0;
}
