#include "flora.h"
#include "generate_xdc.h"
#include "fpga.h"
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

flora::flora(input_to_flora *input_fl)
{

    flora_input = input_fl;

#ifdef WITH_PARTITIONING
    if(flora_input->num_rm_modules > 0) {
        num_rm_modules = flora_input->num_rm_modules;
#else
    if(flora_input->num_rm_partitions > 0) {
        num_rm_partitions = flora_input->num_rm_partitions;
#endif
//        type = flora_input->type_of_fpga; 

#ifdef WITH_PARTITIONING
        platform = new Platform(3);
        task_set = new Taskset(num_rm_modules, num_rm_modules, *platform);
#endif

#ifdef WITH_PARTITIONING 
        cout << "FLORA: num of slots **** " << num_rm_modules <<endl;
#else        
        cout << "FLORA: num of partitions **** " << num_rm_partitions <<endl;
#endif
//        cout << "FLORA: type of FPGA **** " << type <<endl;
        cout << "FLORA: path for input **** " << flora_input->path_to_input <<endl;
    } 
    else {
        cout <<"FLORA: The number of Reconfigurable modules > 0";
        exit(-1);
    }
}

flora::~flora()
{
    cout << "FLORA: destruction " << endl;
}

//Prepare the input
void flora::clear_vectors()
{
    clb_vector.clear();
    bram_vector.clear();
    dsp_vector.clear();

    eng_x.clear();
    eng_y.clear();
    eng_w.clear();
    eng_h.clear();

    x_vector.clear();
    y_vector.clear();
    w_vector.clear();
    h_vector.clear();
}

void flora::prep_input()
{
    unsigned long row, col;
    int i , k;
    unsigned int ptr;
    string str;
    CSVData csv_data(flora_input->path_to_input);

    row = csv_data.rows();
    col = csv_data.columns();

    cout << endl << "FLORA: resource requirement of the input slots " <<endl;
    cout << "\t clb " << " \t bram " << "\t dsp " <<endl;
#ifdef WITH_PARTITIONING 
    for(i = 0, ptr = 0, k = 0; i < num_rm_modules; i++, ptr++) {
#else    
    for(i = 0, ptr = 0, k = 0; i < num_rm_partitions; i++, ptr++) {
#endif
        cout << "slot " << i;  
        str = csv_data.get_value(i, k++);
        clb_vector[ptr] = std::stoul(str);

        str = csv_data.get_value(i, k++);
        bram_vector[ptr] = std::stoul(str);

        str = csv_data.get_value(i, k++);
        dsp_vector[ptr] = std::stoul(str);

#ifdef WITH_PARTITIONING
        str = csv_data.get_value(i, k++);
        HW_WCET[ptr] = std::stod(str);

        str = csv_data.get_value(i, k++);
        slacks[ptr] = std::stod(str);     
#endif
//        cell_name[i] = csv_data.get_value(i, k++);
        k = 0;

        cout << "\t " << clb_vector[ptr] << "\t " << bram_vector[ptr] << "\t " 
             << dsp_vector[ptr] << endl;
    }
}

void flora::start_optimizer()
{
    int i;
    param.bram = &bram_vector;
    param.clb  = &clb_vector;
    param.dsp  = &dsp_vector;
#ifdef WITH_PARTITIONING
    param.num_rm_modules = num_rm_modules;
#else
    param.num_rm_partitions = num_rm_partitions;
#endif

    param.num_connected_slots = connections;
    param.conn_vector = &connection_matrix;

#ifdef WITH_PARTITIONING    
    for(i = 0; i < num_rm_modules; i++){
        task_set->HW_Tasks[i].resDemand[CLB]  = clb_vector[i];
        task_set->HW_Tasks[i].resDemand[BRAM] = bram_vector[i];
        task_set->HW_Tasks[i].resDemand[DSP]  = dsp_vector[i];
        task_set->HW_Tasks[i].WCET = HW_WCET[i];
        task_set->SW_Tasks[i].H.push_back(i);
        task_set->HW_Tasks[i].SW_Task_ID = i;
    }

    double alpha = 2.0;
    double tmp2[] = { alpha, alpha, alpha};
    vector<double> res_usage(tmp2, tmp2+3 );
    
    task_set->print();

    //slacks = generate_slacks(*task_set, *platform, utilization);
    param.task_set = task_set;
    param.platform = platform;
    param.slacks = &slacks;
#endif

//if(type == TYPE_ZYNQ){
#ifdef FPGA_ZYNQ
    zynq = new zynq_7010();
    for(i = 0; i < zynq->num_forbidden_slots; i++) {
        forbidden_region[i] = zynq->forbidden_pos[i];
    //cout<< " fbdn" << forbidden_region[i].x << endl;
    }

    param.num_forbidden_slots = zynq->num_forbidden_slots;
    param.num_rows = zynq->num_rows;
    param.width = zynq->width;
    param.fbdn_slot = &forbidden_region;
    param.num_clk_regs  = zynq->num_clk_reg /2;
    param.clb_per_tile  = ZYNQ_CLB_PER_TILE;
    param.bram_per_tile = ZYNQ_BRAM_PER_TILE;
    param.dsp_per_tile  = ZYNQ_DSP_PER_TILE;

#ifdef WITH_PARTITIONING
    platform->maxFPGAResources[CLB]  = ZYNQ_CLB_TOT;
    platform->maxFPGAResources[BRAM] = ZYNQ_BRAM_TOT;
    platform->maxFPGAResources[DSP]  = ZYNQ_DSP_TOT;

    platform->recTimePerUnit[CLB]  = 1.0/1500.0;
    platform->recTimePerUnit[BRAM] = 1.0/1500.0;
    platform->recTimePerUnit[DSP]  = 1.0/1000.0;
#endif
    cout <<"FLORA: starting ZYNQ MILP optimizer " <<endl;
    zynq_start_optimizer(&param, &from_solver);
    cout <<"FLORA: finished MILP optimizer " <<endl;

//    }

#elif FPGA_PYNQ
    pynq_inst = new pynq();
    for(i = 0; i < pynq_inst->num_forbidden_slots; i++) {
        forbidden_region[i] = pynq_inst->forbidden_pos[i];
    //cout<< " fbdn" << forbidden_region[i].x << endl;
    }   

    param.num_forbidden_slots = pynq_inst->num_forbidden_slots;
    param.num_rows = pynq_inst->num_rows;
    param.width = pynq_inst->width;
    param.fbdn_slot = &forbidden_region;
    param.num_clk_regs  = pynq_inst->num_clk_reg /2; 
    param.clb_per_tile  = PYNQ_CLB_PER_TILE;
    param.bram_per_tile = PYNQ_BRAM_PER_TILE;
    param.dsp_per_tile  = PYNQ_DSP_PER_TILE;

#ifdef WITH_PARTITIONING
    platform->maxFPGAResources[CLB]  = PYNQ_CLB_TOT;
    platform->maxFPGAResources[BRAM] = PYNQ_BRAM_TOT;
    platform->maxFPGAResources[DSP]  = PYNQ_DSP_TOT;

    platform->recTimePerUnit[CLB]  = 1.0/4500.0;
    platform->recTimePerUnit[BRAM] = 1.0/4500.0;
    platform->recTimePerUnit[DSP]  = 1.0/4000.0;
#endif
    cout <<"FLORA: starting PYNQ MILP optimizer " <<endl;
    pynq_start_optimizer(&param, &from_solver);
    cout <<"FLORA: finished MILP optimizer " <<endl;
#endif  
}


void flora::generate_cell_name(unsigned long num_part, vector<std::string> *cell)
{
    int i;
    for(i = 0; i < num_part; i++)
        (*cell)[i] = "dart_i/acc_" + to_string(i) + "/inst";

}

void flora::generate_xdc(std::string fplan_xdc_file)
{
    param_from_solver *from_sol_ptr = &from_solver;

#ifdef FPGA_ZYNQ
    zynq_fine_grained *fg_zynq_instance = new zynq_fine_grained();
#ifdef WITH_PARTITIONING
    generate_cell_name(from_solver.num_partition, &cell_name);
    generate_xdc_file(fg_zynq_instance, from_sol_ptr, param, from_solver.num_partition, cell_name, fplan_xdc_file);
#else
    generate_cell_name(num_rm_partitions, &cell_name);
    generate_xdc_file(fg_zynq_instance, from_sol_ptr, param, num_rm_partitions, cell_name, fplan_xdc_file);
#endif
    
#elif FPGA_PYNQ
    pynq_fine_grained *fg_pynq_instance = new pynq_fine_grained();
#ifdef WITH_PARTITIONING
    generate_cell_name(from_solver.num_partition, &cell_name);
    generate_xdc_file(fg_pynq_instance, from_sol_ptr, param, from_solver.num_partition, cell_name, fplan_xdc_file);
#else
    generate_cell_name(num_rm_partitions, &cell_name);
    generate_xdc_file(fg_pynq_instance, from_sol_ptr, param, num_rm_partitions, cell_name, fplan_xdc_file);
#endif
#endif
}

#ifdef WITH_PARTITIONING
vector<unsigned long> flora::get_units_per_task(unsigned long n, unsigned long n_units, unsigned long n_min, unsigned long n_max)
{
    vector<unsigned long> ret;
    double rand_dbl;

    uint n_units_sum = n_units, n_units_next=0;

    for(uint i=0; i < n-1; i++)
    {
        srand(time(0));
        rand_dbl = pow(MY_RAND(),(1.0 / (double)(n - i - 1)));
        n_units_next = floor((double)n_units_sum * rand_dbl);
        //cout << n_units_next << " " << rand_dbl << endl;
        // --------- LIMIT Task Utilization --------------
        if(n_units_next > (n_units_sum - n_min))
            n_units_next = n_units_sum - n_min;

        if(n_units_next < ((n - i - 1) * n_min))
            n_units_next = (n - i - 1) * n_min;

        if((n_units_sum - n_units_next) > n_max)
            n_units_next = n_units_sum - n_max;
        // ------------------------------------------------

        ret.push_back(n_units_sum - n_units_next);
        n_units_sum = n_units_next;
    }

    ret.push_back(n_units_sum);
    return ret;
}

vector <double> flora::generate_slacks(Taskset& t, Platform& platform, double alpha)
{
    std::vector<double> maxSlotSizePerResource;

      for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
      {
          double tmp = 0;
          for(uint a=0; a < t.maxHW_Tasks; a++)
              tmp = max(tmp, t.HW_Tasks[a].resDemand[x]);

          maxSlotSizePerResource.push_back(tmp);
          //cout << "MAX_SIZE[" << x << "] = " << tmp << endl;
      }

      std::vector<double> SLACK_BOUND;

      double max_delay = 1;

      for(uint a=0; a < t.maxHW_Tasks; a++)
          for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
               max_delay += maxSlotSizePerResource[x]*platform.recTimePerUnit[x];

      for(uint b=0; b < t.maxHW_Tasks; b++)
          max_delay += t.HW_Tasks[b].WCET;

      //cout << "MAX DELAY = " << max_delay << endl;

      for(uint a=0; a < t.maxHW_Tasks; a++)
      {
          const double min_delay = t.HW_Tasks[a].WCET + alpha*(max_delay-t.HW_Tasks[a].WCET);
          SLACK_BOUND.push_back(min_delay + (max_delay-min_delay)*MY_RAND());
          cout <<"slack " <<a << " " <<SLACK_BOUND[a] <<endl;
      }

      return SLACK_BOUND;
}

vector<uint> get_units_per_task(uint n, uint n_units, uint n_min, uint n_max)
{
    vector<uint> ret;

    uint n_units_sum = n_units, n_units_next=0;

    for(uint i=0; i < n; i++)
    {

        n_units_next = floor((double)n_units_sum * pow(MY_RAND(),(1.0 / (double)(n - i - 1))));

        // --------- LIMIT Task Utilization --------------
        if(n_units_next>(n_units_sum-n_min))
            n_units_next = n_units_sum-n_min;

        if(n_units_next<((n-i-1)*n_min))
            n_units_next = (n-i-1)*n_min;

        if((n_units_sum - n_units_next)>n_max)
            n_units_next = n_units_sum-n_max;
        // ------------------------------------------------


        ret.push_back(n_units_sum-n_units_next);
        n_units_sum =  n_units_next;
    }

//    ret.push_back(n_units_sum);

    return ret;
}

vector<HW_Task_t> generate_HW_tasks
                  (uint n, Platform& p,
                   const vector<double>& res_usage,
                   double WCET_area_ratio,
                   double max_area_usage)
{
    const uint n_res = p.maxFPGAResources.size();
    uint temp[5] = { 10,10,0};

    vector<HW_Task_t> HW_Tasks;

        //cout <<"in generate HW task "<< p.maxFPGAResources.size() << " " << res_usage.size() <<endl;
    if(res_usage.size()!=n_res)
        return HW_Tasks;

    // Prepare task set
    for(uint i=0; i < n; i++)
    {
        HW_Task_t new_task(p);
        HW_Tasks.push_back(new_task);
    }

    // For each FPGA resource
    for(uint x=0; x < n_res; x++)
    {
        const uint n_units = floor(res_usage[x]*p.maxFPGAResources[x]);

        vector<uint> res_per_task = get_units_per_task(n, n_units, temp[x],
                                                       max_area_usage*p.maxFPGAResources[x]);

        cout << endl << endl << "pritning res reqmt " <<endl;
        for(uint i=0; i < n; i++){
            HW_Tasks[i].resDemand[x] = res_per_task[i];
            //cout << " Res"<<i<< " " <<res_per_task[i];
        }

        cout <<endl;
    }

    // For each HW-task
    for(uint i=0; i < n; i++)
    {
        uint acc = 0;
        for(uint x=0; x < n_res; x++)
            acc += (uint)HW_Tasks[i].resDemand[x];
        //FIXME
        //const uint lb = (uint)floor(acc/WCET_area_ratio*(1.0-VAR));
        //const uint ub = (uint)floor(acc/WCET_area_ratio*(1.0+VAR));
        const uint lb   = 5;
        const uint ub   = 500;
        HW_Tasks[i].WCET = lb + (ub-lb)*MY_RAND();

        cout <<"WCET " <<i <<  HW_Tasks[i].WCET <<endl;
    }

    return HW_Tasks;
}

Taskset flora::generate_taskset_one_HW_task_per_SW_task(uint n, Platform& p,
                                                        const vector<double>& res_usage,
                                                        double WCET_area_ratio,
                                                        double max_area_usage)

{
    Taskset t(n, n, p);
    vector<HW_Task_t> HW_Tasks = generate_HW_tasks(n, p, res_usage, WCET_area_ratio, max_area_usage);

    if(HW_Tasks.size()!=n)
        return t;

    // Fill the task set with the HW-tasks

    for(uint i=0; i< n; i++){
        t.HW_Tasks[i] = HW_Tasks[i];
        // i=th HW-Task <---> i-th SW-Task
        t.SW_Tasks[i].H.push_back(i);
        t.HW_Tasks[i].SW_Task_ID = i;
    }

    cout <<"in generate HW task" <<endl;
    return t;
}
#endif
