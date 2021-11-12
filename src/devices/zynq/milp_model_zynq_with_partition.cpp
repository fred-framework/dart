#include "gurobi_c++.h"
#include "milp_solver_interface.h"
#include "fpga.h"

using namespace std;
//Gurobi data types

typedef vector<GRBVar>          GRBVarArray;
typedef vector<GRBVarArray>     GRBVar2DArray;
typedef vector<GRBVar2DArray>   GRBVar3DArray;
typedef vector<GRBVar3DArray>   GRBVar4DArray;

static unsigned long H, W;
static unsigned long num_slots;
static unsigned long num_rows;
static unsigned long num_forbidden_slots;
static unsigned long BIG_M = 10000000;
static unsigned long num_clk_regs;

unsigned long wasted_clb_zynq , wasted_bram_zynq , wasted_dsp_zynq ;

static int status;
static unsigned long delta_size;

static unsigned long clb_max = 10000;
static unsigned long bram_max = 10000;
static unsigned long dsp_max = 10000;
static unsigned long clb_per_tile;
static unsigned long bram_per_tile;
static unsigned long dsp_per_tile;

static vector<unsigned long> clb_req_zynq (MAX_SLOTS);
static vector<unsigned long> bram_req_zynq(MAX_SLOTS);
static vector<unsigned long> dsp_req_zynq (MAX_SLOTS);

static vector<double> clb_in_partition(MAX_SLOTS);
static vector<double> bram_in_partition(MAX_SLOTS);
static vector<double> dsp_in_partition(MAX_SLOTS);

Taskset *task_set;
Platform *platform;
vector <double> slacks = vector<double>(MAX_SLOTS);

static vector <vector <unsigned long>> conn_matrix_zynq = vector <vector<unsigned long>> (MAX_SLOTS, vector<unsigned long> (MAX_SLOTS, 0));;
static unsigned long num_conn_slots_zynq;

const unsigned long num_fbdn_edge = 7;
static Vecpos fs_zynq(MAX_SLOTS);
unsigned int beta_fbdn[3] = {1, 1};
unsigned forbidden_boundaries_right[num_fbdn_edge] = {4, 7, 10, 15, 18, 22, 25};
unsigned forbidden_boundaries_left[num_fbdn_edge] =  {3, 6, 9, 12, 17, 21, 24};

//int solve_milp(param_from_solver *to_sim)
int solve_milp(Taskset &t, Platform &platform, vector<double> &slacks, bool preemptive_FRI, param_from_solver *to_sim)
{
    unsigned long status, i ,k, j, l, m;
    unsigned long dist_0, dist_1, dist_2;
    unsigned long num_active_partitions = 0;
    
    //define variables
    try {
        GRBEnv env = GRBEnv();
        GRBConstr* c = NULL;
        GRBModel model = GRBModel(env);

        if(num_slots >= num_forbidden_slots)
            delta_size = num_slots;
        else
            delta_size = num_forbidden_slots;

        //Variable definition
       //Variables for partioning
        // ------------------------------------------------------------
        // Variable defintion: b
        // Meaning: b[x][k] = number of FPGA resources of type 'x',
        //          in the 'k'-th partition
        // ------------------------------------------------------------

        GRBVar2DArray b(platform.N_FPGA_RESOURCES);
        for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
        {
            GRBVarArray per_partition(t.maxPartitions);
            b[x] = per_partition;

            for(uint k=0; k < t.maxPartitions; k++)
                b[x][k] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }


        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: A
        // Meaning: A[a][k] == 1 iff the 'a'-th HW-task is allocated to
        //          the the 'k'-th partition
        // ------------------------------------------------------------

        GRBVar2DArray A(t.maxHW_Tasks);
        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            GRBVarArray per_partition(t.maxPartitions);
            A[a] = per_partition;

            for(uint k=0; k < t.maxPartitions; k++)
                A[a][k] = model.addVar(0.0, 1.0, 0.0, GRB_INTEGER);
        }

        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: gamma
        // Meaning: gamma[a]==1 iff the 'a'-th HW-task is subject to DPR
        // ------------------------------------------------------------

        GRBVarArray gamma_part(t.maxHW_Tasks);
        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            gamma_part[a] = model.addVar(0.0, 1.0, 0.0, GRB_INTEGER);
        }

        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: I_SLOT
        // Meaning: I_SLOT[a][b] = interference caused by 'b'-th HW-Task
        // to the 'a'-th HW-Task due to slot contention
        // ------------------------------------------------------------

        GRBVar2DArray I_SLOT(t.maxHW_Tasks);
        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            GRBVarArray per_other_HWTask(t.maxHW_Tasks);
            I_SLOT[a] = per_other_HWTask;

            for(uint b=0; b < t.maxHW_Tasks; b++)
                I_SLOT[a][b] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }

        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: DELTA
        // Meaning: DELTA[a][i] = interference caused by HW-Tasks used
        // by the 'i'-th SW-Task to the 'a'-th HW-Task
        // ------------------------------------------------------------

        GRBVar2DArray DELTA(t.maxHW_Tasks);
        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            GRBVarArray per_SWTask(t.maxSW_Tasks);
            DELTA[a] = per_SWTask;

            for(uint i=0; i < t.maxSW_Tasks; i++)
                DELTA[a][i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }

        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: r
        // Meaning: r[a] = reconfiguration time of 'a'-th HW-Task
        // ------------------------------------------------------------

        GRBVarArray r(t.maxHW_Tasks);
        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            r[a] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        }


        // ------------------------------------------------------------
        // ------------------------------------------------------------
        // Variable defintion: DELTA_NP
        // Meaning: DELTA_NP[a][b] = interference due to non-preemptive
        // reconfiguration *directly* incurred by the 'b'-th HW-Task
        // during a request for the 'a'-th HW-Task
        // ------------------------------------------------------------

        GRBVar2DArray DELTA_NP(t.maxHW_Tasks);
        if(!preemptive_FRI)
        {
            for(uint a=0; a < t.maxHW_Tasks; a++)
            {
                GRBVarArray per_other_HWTask(t.maxHW_Tasks);
                DELTA_NP[a] = per_other_HWTask;

                for(uint b=0; b < t.maxHW_Tasks; b++)
                    DELTA_NP[a][b] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            }
        }

        /**********************************************************************
         name: x
         type: integer
         func: x[i][k] represent the left and right x coordinate of slot 'i'
        ***********************************************************************/

        GRBVar2DArray x(num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(2);
            x[i] = each_slot;

            for(k = 0; k < 2; k++)
                x[i][k] = model.addVar(0.0, W, 0.0, GRB_INTEGER);
        }

        /**********************************************************************
         name: y
         type: integer
         func: y[i] represents the bottom left y coordinate of slot 'i'
        ***********************************************************************/

        GRBVarArray y (num_slots);
        for(i = 0; i < num_slots; i++) {
            y[i] = model.addVar(0.0, num_clk_regs, 0.0, GRB_INTEGER);
        }

        /**********************************************************************
         name: w
         type: integer
         func: w[i] represents the width of the slot 'i'
        ***********************************************************************/

        GRBVarArray w (num_slots);
        for(i = 0; i < num_slots; i++) {
            w[i] = model.addVar(0.0, W, 0.0, GRB_INTEGER);
        }

        /**********************************************************************
         name: h
         type: integer
         func: h[i] represents the height of slot 'i'
        ***********************************************************************/

        GRBVarArray h (num_slots);
        for(i = 0; i < num_slots; i++) {
            h[i] = model.addVar(0.0, H, 0.0, GRB_INTEGER);
        }

        /**********************************************************************
         name: z
         type: binary
         func: z[i][k][][] is used to define the constraints on the distribution of
               resource on the FPGA fabric

               z[0][i][x_1/2][] == for clb
               z[1][i][x_1/2][] == for bram
               z[2][i][x_1/2][] == for dsp
        ***********************************************************************/

        GRBVar4DArray z(6);
        for(i = 0; i < 6; i++) {
            GRBVar3DArray each_slot(num_slots);
            z[i] = each_slot;

            for(k = 0; k < num_slots; k++)  {
                GRBVar2DArray x_coord(2);
                z[i][k] = x_coord;

                for(j = 0; j < 2; j++) {
                    GRBVarArray constrs(200);
                    z[i][k][j] = constrs;

                    for(l = 0; l < 200; l++)
                        z[i][k][j][l] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
                }
            }
        }

       /**********************************************************************
         name: clb
         type: integer
         func: clb[i][k] represents the number of clbs in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total numbe clb in slot 'i' is then calculated by
                clb in 'i' = clb[i][1] - clb[i][0]
        ***********************************************************************/

        GRBVar2DArray clb (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(2);
            clb[i] = each_slot;

            for(k = 0; k < 2; k++)
                clb[i][k] = model.addVar(0.0, clb_max, 0.0, GRB_INTEGER);
        }

       /**********************************************************************
         name: bram
         type: integer
         func: bram[i][k] represents the number of brams in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total numbe brams in slot 'i' is then calculated by
                bram in 'i' = bram[i][1] - bram[i][0]
        ***********************************************************************/

        GRBVar2DArray bram (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(2);
            bram[i] = each_slot;

            for(k = 0; k < 2; k++)
                bram[i][k] = model.addVar(0.0, bram_max, 0.0, GRB_INTEGER);
        }
//#ifdef dspp
        /**********************************************************************
         name: dsp
         type: integer
         func: dsp[i][k] represents the number of dsp in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total numbe brams in slot 'i' is then calculated by
                dsp in 'i' = dsp[i][1] - dsp[i][0]
        ***********************************************************************/

        GRBVar2DArray dsp (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(2);
            dsp[i] = each_slot;

            for(k = 0; k < 2; k++)
                dsp[i][k] = model.addVar(0.0, dsp_max, 0.0, GRB_INTEGER);
        }

       /**********************************************************************
         name: clb-fbdn
         type: integer
         func: clb_fbdn[i][k] represents the number of clbs in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total number of clb in forbidden region 'i' is then calculated by
                clb_fbdn in 'i' = clb_fbdn[i][1] - clb_fbdn[i][0]
        ***********************************************************************/
        GRBVar3DArray clb_fbdn (2); //TODO: This should be modified to num_forbidden_slots
        for(i= 0; i < 2; i++) {
            GRBVar2DArray each_slot(num_slots);
            clb_fbdn[i] = each_slot;

            for(j = 0; j < num_slots; j++) {
                GRBVarArray each_slot_fbdn(2);

                clb_fbdn[i][j] = each_slot_fbdn;
                for(k = 0; k < 2; k++)
                    clb_fbdn[i][j][k] = model.addVar(0.0, clb_max, 0.0, GRB_INTEGER);
            }
        }

        /**********************************************************************
        The following 3 variables record the total number of CLB, BRAM and DSP 
        in forbidden regions.
        ***********************************************************************/

        GRBVarArray clb_fbdn_tot (num_slots);
        for(i = 0; i < num_slots; i++) {
            clb_fbdn_tot[i] = model.addVar(0, clb_max, 0.0, GRB_INTEGER);
        }

        GRBVarArray bram_fbdn_tot (num_slots);
        for(i = 0; i < num_slots; i++) {
            bram_fbdn_tot[i] = model.addVar(0, bram_max, 0.0, GRB_INTEGER);
        }

        GRBVarArray dsp_fbdn_tot (num_slots);
        for(i = 0; i < num_slots; i++) {
            dsp_fbdn_tot[i] = model.addVar(0, dsp_max, 0.0, GRB_INTEGER);
        }


       /**********************************************************************
         name: bram_fbdn
         type: integer
         func: bram[i][k] represents the number of brams in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total number of brams in the forbidden region 'i' is then calculated by
                bram in 'i' = bram_fbdn[i][1] - bram_fbdn[i][0]
        ***********************************************************************/
        GRBVar3DArray bram_fbdn (2);
        for(j = 0; j < 2; j++) {
            GRBVar2DArray each_slot (num_slots);
            bram_fbdn[j] = each_slot;

            for(i = 0; i < num_slots; i++) {
                GRBVarArray each_slot_fbdn(2);
                bram_fbdn[j][i] = each_slot_fbdn;

                for(k = 0; k < 2; k++)
                    bram_fbdn[j][i][k] = model.addVar(0.0, bram_max, 0.0, GRB_INTEGER);
            }
    }
        /**********************************************************************
         name: dsp_fbdn
         type: integer
         func: dsp_fbdn[i][k] represents the number of dsp in (0, x_1) & (0, x_2)
               in a single row.
               'k' = 0 -> x_1
               'k' = 1 -> x_2

               the total numbe dsps in forbidden region 'i' is then calculated by
                dsp in 'i' = dsp_fbdn[i][1] - dsp_fbdn[i][0]
        ***********************************************************************/
        GRBVar3DArray dsp_fbdn (2);
        for(j = 0; j < 2; j++) {
            GRBVar2DArray each_fbdn_slot (num_slots);
            dsp_fbdn[j] = each_fbdn_slot;

            for(i = 0; i < num_slots; i++) {
                GRBVarArray each_slot(2);
                dsp_fbdn[j][i] = each_slot;

                for(k = 0; k < 2; k++)
                    dsp_fbdn[j][i][k] = model.addVar(0.0, dsp_max, 0.0, GRB_INTEGER);
            }
        }

//#endif
        /**********************************************************************
         name: beta
         type: binary
         func: beta[i][k] = 1 if clock region k is part of slot 'i'
        ***********************************************************************/

        GRBVar2DArray beta (num_slots);
        for(i = 0; i < num_slots; i++) {
                GRBVarArray for_each_clk_reg(num_clk_regs);
                beta[i] = for_each_clk_reg;

                for(j = 0; j < num_clk_regs; j++) {
                     beta[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
            }
        }

        /**********************************************************************
         name: tau
         type: integer
         func: tau[i][k] is used to linearize the function which is used to compute
               the number of available resources. The first index is used to
               denote the type of resource and the second is used to denote
               the slot
        ***********************************************************************/
        GRBVar3DArray tau (3); //for clb, bram, dsp
        for(i = 0; i < 3; i++) {
            GRBVar2DArray each_slot(num_slots);
            tau[i] = each_slot;

            for(l = 0; l < num_slots; l++) {
                GRBVarArray for_each_clk_reg(num_clk_regs);

                tau[i][l] = for_each_clk_reg;
                for(k = 0; k < num_clk_regs; k++) {
                      tau[i][l][k] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER);
                }
            }
        }

        /**********************************************************************
         name: tau_fbdn
         type: integer
         func: tau_fbdn[i][k] is used to linearize the function which is used to compute
               the number of available resources. The first index is used to
               denote the type of resource and the second is used to denote
               the slot
        ***********************************************************************/
        GRBVar4DArray tau_fbdn (2); //for forbidden clb, bram, dsp
        for(j = 0; j < 2; j++) {
            GRBVar3DArray each_slot_fbdn(3);
            tau_fbdn[j] = each_slot_fbdn;

            for(i=0; i < 3; i++) {
                GRBVar2DArray each_slot(num_slots);
                tau_fbdn[j][i] = each_slot;

                for(l = 0; l < num_slots; l++) {
                    GRBVarArray for_each_clk_reg(num_clk_regs);

                    tau_fbdn[j][i][l] = for_each_clk_reg;
                    for(k = 0; k < num_clk_regs; k++) {
                        tau_fbdn[j][i][l][k] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER);
                    }
                }
            }
        }



       /**********************************************************************
         name: gamma
         type: binary
         func: gamma[i][k] = 1 iff bottom left x coordinate of slot 'i' is found
               to the left of the bottom left x coordinate of slot 'k'

               gamma[i][k] = 1 if x_i <= x_k
        ***********************************************************************/

        GRBVar2DArray gamma (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);
            gamma[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                gamma[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: theta
         type: binary
         func: theta[i][k] = 1 iff the bottom left y coordinate of slot 'i' is
               found below the bottom left y coordinate of slot 'k'

               theta[i][k] = 1 if y_i <= y_k
        ***********************************************************************/

        GRBVar2DArray theta (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);
            theta[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                theta[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: Gamma
         type: binary
         func: Gamma[i][k] = 1 iff x_i + w_i >= x_k
        ***********************************************************************/

        GRBVar2DArray Gamma (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);

            Gamma[i] = each_slot;
            for(k = 0; k < num_slots; k++)
                Gamma[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: Alpha
         type: binary
         func: Alpha[i][k] = 1 iff x_k + w_k >= x_i
        ***********************************************************************/

        GRBVar2DArray Alpha (num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);
            Alpha[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                Alpha[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }


        /**********************************************************************
         name: Omega
         type: binary
         func: Omega[i][k] = 1 iff y_i + h_i >= y_k
        ***********************************************************************/

        GRBVar2DArray Omega(num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);
            Omega[i] = each_slot;

           for(k = 0; k < num_slots; k++)
                Omega[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: Psi
         type: binary
         func: Psi[i][k] = 1 iff  y_k + h_k >= y_i
        ***********************************************************************/

        GRBVar2DArray Psi(num_slots);
        for(i = 0; i < num_slots; i++) {
            GRBVarArray each_slot(num_slots);
            Psi[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                Psi[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: delta
         type: binary
         func: delta[i][k] = 1 if slot 'i' and 'k' share at least one tile
        ***********************************************************************/

        GRBVar3DArray delta (2);
        for(j = 0; j < 2; j++) {
            GRBVar2DArray each_slot(delta_size);

            delta[j] = each_slot;
            for(i = 0; i < delta_size; i++) {
                GRBVarArray each_slot_1(delta_size);

                delta[j][i] = each_slot_1;
                for(k = 0; k < delta_size; k++)
                    delta[j][i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
            }
        }
//#ifdef fbdn

        /**********************************************************************
         name: mu
         type: binary
         func: mu[i][k] = 1 iff bottom left x coordinate of slot 'i' is found
               to the left of the bottom left x coordinate of slot 'k'

               mu[i][k] = 1 if x_i <= x_k
        ***********************************************************************/

        GRBVar2DArray mu (num_forbidden_slots);
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);
            mu[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                mu[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: nu
         type: binary
         func: nu[i][k] = 1 iff bottom left x coordinate of slot 'i' is found
               to the left of the bottom left x coordinate of slot 'k'

               nu[i][k] = 1 if x_i <= x_k
        ***********************************************************************/
        GRBVar2DArray nu (num_forbidden_slots);
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);
            nu[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                nu[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************https://www.youtube.com/watch?v=os0D15JkfAk
         name: fbdn_1
         type: binary
         func: fbdn_1[i][k] = 1 iff forbidden slot 'i' x variable interferes
               with slot 'k'
        ***********************************************************************/

        GRBVar2DArray fbdn_1 (num_forbidden_slots);
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);

            fbdn_1[i] = each_slot;
            for(k = 0; k < num_slots; k++)
                fbdn_1[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: fbdn_2
         type: binary
         func: fbdn_2[i][k] = 1 iff forbidden slot 'i' x variable interferes
               with slot 'k'
        ***********************************************************************/

        GRBVar2DArray fbdn_2 (num_forbidden_slots) ;
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);

            fbdn_2[i] = each_slot;
            for(k = 0; k < num_slots; k++)
                fbdn_2[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: fbdn_3
         type: binary
         func: fbdn_3[i][k] = 1 iff forbidden slot 'i' x variable interferes
               with slot 'k'
        ***********************************************************************/

        GRBVar2DArray fbdn_3 (num_forbidden_slots);
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);

            fbdn_3[i] = each_slot;
            for(k = 0; k < num_slots; k++)
                fbdn_3[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        /**********************************************************************
         name: fbdn_4
         type: binary
         func: fbdn_4[i][k] = 1 iff forbidden slot 'i' x variable interferes
               with slot 'k'
        ***********************************************************************/
        GRBVar2DArray fbdn_4 (num_forbidden_slots);
        for(i = 0; i < num_forbidden_slots; i++) {
            GRBVarArray each_slot(num_slots);

            fbdn_4[i] = each_slot;
            for(k = 0; k < num_slots; k++)
                fbdn_4[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }
//#endif
        /**********************************************************************
           name: kappa
           type: binary
           func: this variable is used to formulate the constraint on wasted resources
                  kappa[i][k] is a variable to constrain wasted resource type i in slot k
          ***********************************************************************/
          GRBVar3DArray kappa(num_slots);
          for(i = 0; i < num_slots; i++) {
              GRBVar2DArray each_slot(num_fbdn_edge);

              kappa[i] = each_slot;
              for(k = 0; k < num_fbdn_edge; k++){
                  GRBVarArray each_slot_1 (2);
                    kappa[i][k] = each_slot_1;

                  for(j = 0; j < 2; j++)
                    kappa[i][k][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
                }
            }


        /**********************************************************************
         name: constr_res
         type: binary
         func: this variable is used to formulate the constraint on wasted resources
                constr_res[i][k] is a variable to constrain wasted resource type i in slot k
        ***********************************************************************/
  /*      GRBVar2DArray res_constr(2);
        for(i = 0; i < 2; i++) {
            GRBVarArray each_slot(num_slots);

            res_constr[i] = each_slot;

            for(k = 0; k < num_slots; k++)
                res_constr[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        cout<< "defining variables tau " <<endl;
*/

        /**********************************************************************
          name: centroid
          type: integer
          func: centroid[i][k] represents the centroid of a slot i.
                centroid[i]0] is the centroid on the x axis while
                centroid [i][1] is the centroid on the y axis
         ***********************************************************************/

         GRBVar2DArray centroid (num_slots);
         for(i = 0; i < num_slots; i++) {
             GRBVarArray each_slot(2);
             centroid[i] = each_slot;

             for(k = 0; k < 2; k++)
                 centroid[i][k] = model.addVar(0.0,  GRB_INFINITY, 0.0, GRB_INTEGER);
         }

         /**********************************************************************
           name: dist
           type: integer
           func: this variable represents the distance between the centroids of
                 two regions. dist[i][k][0] represents the distance on the x axis
                 between slots i and slot k while dist[i][k][1] represents the
                 distance on the y axis between slots i and k.
          ***********************************************************************/
         GRBVar3DArray dist (num_slots);
         for(j = 0; j < num_slots; j++) {
             GRBVar2DArray each_slot(num_slots);

             dist[j] = each_slot;
             for(i = 0; i < num_slots; i++) {
                 GRBVarArray each_slot_1(2);

                 dist[j][i] = each_slot_1;
                 for(k = 0; k < 2; k++)
                     dist[j][i][k] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER);
             }
         }

         /**********************************************************************
           name: wasted
           type: integer
           func: centroid[i][k] represents the wasted resources in each slot
                 k = 0 => clb
                 k = 1 => bram
                 k = 2 => dsp
          ***********************************************************************/

          GRBVar2DArray wasted (num_slots);
          for(i = 0; i < num_slots; i++) {
              GRBVarArray each_slot(3);
              wasted[i] = each_slot;

              for(k = 0; k < 3; k++)
                  wasted[i][k] = model.addVar(0.0,  GRB_INFINITY, 0.0, GRB_INTEGER);
          }

         /**********************************************************************
           name: max
           type: binary
           func: 
           ***********************************************************************/

        GRBVar2DArray max(t.maxPartitions);
        for(uint a=0; a < t.maxPartitions; a++)
        {
            GRBVarArray per_partition(t.maxHW_Tasks);
            max[a] = per_partition;

            for(uint k=0; k < t.maxHW_Tasks; k++)
                max[a][k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }


        model.update();


        /********************************************************************
         Constraint 0.1: Every HW-Task must be allocated somewhere
       ***********************************************************************/

        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            GRBLinExpr exp;

            for(uint k=0; k < t.maxPartitions; k++)
                exp += A[a][k];

            model.addConstr(exp == 1, "con 1");
        }

        /********************************************************************
         Constraint 0.2: Decide wether a HW-task is subject to DPR
        ***********************************************************************/

        for(uint a=0; a < t.maxHW_Tasks; a++)
            for(uint b=0; b < t.maxHW_Tasks; b++)
            {
                if(a==b) continue;

                for(uint k=0; k < t.maxPartitions; k++)
                    model.addConstr(gamma_part[a] >= A[b][k] -(1-A[a][k]), "con 2");
            }

        /********************************************************************
         Constraint 0.3: Feasibility condition for the resources avaialable
                         on the FPGA
        ***********************************************************************/

        for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
        {
            GRBLinExpr exp;

            for(uint k=0; k < t.maxPartitions; k++)
                    exp += b[x][k];

            model.addConstr(exp <= (double)platform.maxFPGAResources[x], "con 3");
        }

        /********************************************************************
         Constraint 0.4: Each slot must have enough resources to host all the
                        HW-Tasks allocated to its corresponding partition
        ***********************************************************************/

        for(uint x=0; x < platform.N_FPGA_RESOURCES; x++){
            for(uint k=0; k < t.maxPartitions; k++) {
                GRBLinExpr exp, exp_max;
                for(uint a=0; a < t.maxHW_Tasks; a++){
                    model.addConstr(b[x][k] >= (double)t.HW_Tasks[a].resDemand[x]*A[a][k], "con 4");
                    //model.addConstr(b[x][k] <= (double)t.HW_Tasks[a].resDemand[x]*A[a][k] + (1 - max[a][k]) * BIG_M, "con 4_1");
                    exp += A[a][k];
                    exp_max += max[a][k]; 
                }
                //model.addConstr(exp_max == 1, "con 11.0");
                //model.addConstr(b[x][k] <= (double)BIG_M * exp, "con 11");
            }
        }

        /********************************************************************
        Constraint 0.5: Bound the FPGA reconfiguration time
        ***********************************************************************/

        double BIG_M_con5 = 1;
        for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
            BIG_M_con5 += platform.recTimePerUnit[x]*platform.maxFPGAResources[x];

        for(uint a=0; a < t.maxHW_Tasks; a++)
            for(uint k=0; k < t.maxPartitions; k++)
            {
                    GRBLinExpr exp;

                    for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
                        exp += (double)platform.recTimePerUnit[x]*b[x][k];


                   // model.addConstr(r[a] >= exp -(1.0 - A[a][k])*BIG_M_con5
                   //                             -(1.0 - gamma_part[a])*BIG_M_con5, "con 6");
            }
        /********************************************************************
        Constraint 0.6: Interference among HW-tasks due to execution
          ***********************************************************************/

        for(uint k=0; k < t.maxPartitions; k++)
            for(uint a=0; a < t.maxHW_Tasks; a++)
                for(uint b=0; b < t.maxHW_Tasks; b++)
                {
                    // Self-interference is impossible
                    if(a==b) continue;

                    const double WCET = (double)t.HW_Tasks[b].WCET;

                    model.addConstr(I_SLOT[a][b] >= WCET - WCET*(1-A[a][k])
                                                         - WCET*(1-A[b][k]), "con 7");
                }

        /********************************************************************
         Constraint 0.7: Bound on delay incurred when requesting a HW-Task
        ***********************************************************************/

        double BIG_M_con7 = 1;
        for(uint x=0; x < platform.N_FPGA_RESOURCES; x++)
            BIG_M_con7 += platform.recTimePerUnit[x]*platform.maxFPGAResources[x];
        for(uint b=0; b < t.maxHW_Tasks; b++)
            BIG_M_con7 += t.HW_Tasks[b].WCET;

        for(uint a=0; a < t.maxHW_Tasks; a++)
        {
            for(uint i=0; i < t.maxSW_Tasks; i++)
            {
                // Cannot receive interference from HW-tasks used by its SW-Task
                if(t.HW_Tasks[a].SW_Task_ID == i) continue;

                // For each HW-task used by the 'i'-th SW-task...
                for(auto b : t.SW_Tasks[i].H)
                    model.addConstr(DELTA[a][i] >= I_SLOT[a][b] + r[b] -(1-gamma_part[a])*BIG_M_con7, "con 8");

            }
        }
        /********************************************************************
         Constraint 0.8: Enforce (delays <= given bound) to ensure schedulability
        ***********************************************************************/

        for(uint i=0; i < t.maxSW_Tasks; i++)
        {
            GRBLinExpr exp;

            // For each HW-task used by the 'i'-th SW-task...
            for(auto a : t.SW_Tasks[i].H)
            {
                exp += r[a] + t.HW_Tasks[a].WCET;

                for(uint j=0; j < t.maxSW_Tasks; j++)
                {
                    if(i==j) continue;

                    exp += DELTA[a][j];
                }

                if(!preemptive_FRI)
                {
                    for(uint b=0; b < t.maxHW_Tasks; b++)
                        exp += DELTA_NP[a][b];
                }
            }

            model.addConstr(exp <= slacks[i], "con 9");
        }
        
        /********************************************************************
         Constraint 0.9: Bound delay due to non-preemptive reconfiguration
        *********************************************************************/
/*
        if(!preemptive_FRI)
        {
            for(uint a=0; a < t.maxHW_Tasks; a++)
                for(uint b=0; b < t.maxHW_Tasks; b++)
                    for(uint c=0; c < t.maxHW_Tasks; c++)
                        for(uint k=0; k < t.maxPartitions; k++)
                            model.addConstr(DELTA_NP[a][b] >= r[c]
                                            -(2-A[a][k]-A[b][k])*BIG_M_con5
                                            -A[c][k]*BIG_M_con5 -(1-gamma_part[a])*BIG_M_con5
                                            -(1-gamma_part[c])*BIG_M_con5, "con 10");
        }

*/

        /********************************************************************
        Constr 1.1: The x coordinates must be constrained not to exceed
                      the boundaries of the fabric
        ********************************************************************/
        for(i = 0; i < num_slots; i++) {
            //for(k = 0; k< num_slots; k++) {
                model.addConstr(w[i] == x[i][1] - x[i][0], "1");
                //model.addConstr(x[i][0] <= W-1, "2");
                //model.addConstr(x[i][1] <= W, "3");
                model.addConstr(x[i][1] - x[i][0] >= 1, "4");
            //}
        }

        /********************************************************************
        Constr 1.2: The binary variables representing the rows must be
                    contigious i.e, if a region occupies clock region 1 and 3
                    then it must also occupy region 2
        ********************************************************************/
        for(i = 0; i < num_slots; i++) {
             for(j = 0; j < num_clk_regs - 2; j++) {
                 if(num_clk_regs > 2)
                     model.addConstr(beta[i][j+1] >= beta[i][j] + beta[i][j+2] - 1, "5");
             }
         }
/*
        for(i = 0; i < num_slots; i++) {
            GRBLinExpr exp = 0;
            for(j = 0; j < num_clk_regs; j++) {
                //GRBLinExpr exp = 0;
//                for(k = 0; k < num_rows; k++) {
                    exp += beta[i][j];
                }

             for(l = 0; l < num_clk_regs; l++) {
                    model.addConstr(beta[i][l] >= (h[i] - (exp - beta[i][l])), "500");
                }
            }
*/
        /************************************************************************
        Constr 1.3: The height of slot 'i' must be the sum of all clbs in the slot
        *************************************************************************/
        for(i = 0; i < num_slots; i++) {
            GRBLinExpr exp;
            for(j = 0; j < num_clk_regs; j++)
                exp += beta[i][j];
            model.addConstr(h[i] == exp, "6");
        }

        /******************************************************************
        Constr 1.4: y_i must be constrained not to be greater than the
                    lowest row
        ******************************************************************/
        for(i = 0; i < num_slots; i++) {
            GRBLinExpr exp_y;
            for(j = 0; j < num_clk_regs; j++)
                model.addConstr(y[i] <= (H - beta[i][j] * (H - j)), "99");
            model.addConstr(y[i] + h[i] <= H, "100");
        }
        //Resource Constraints
        /******************************************************************
        Constr 2.0: The clb on the FPGA is described using the following
                    piecewise function.
                    x       0  <= x < 4
                    x-1     4  <= x < 7
                    x-2     7  <= x < 10
                    x-3     10 <= x < 15
                    x-4     15 <= x < 18
                    x-5     18 <= x < 22
                    x-6     22 <= x < 25
                    x-7     25 <= x < W

                    The piecewise function is then transformed into a set
                    of MILP constraints using the intermediate variable z
        ******************************************************************/
        for(i =0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                GRBLinExpr exp;
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 4 - x[i][k], "1");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 3, "2");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 7 - x[i][k], "3");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 6, "4");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 10 - x[i][k], "5");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 9,  "6");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 15 - x[i][k], "7");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 14, "8");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 18 - x[i][k], "9");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 17, "10");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 22 - x[i][k], "11");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 21, "12");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= 25 - x[i][k], "13");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= x[i][k] - 24, "14");
                model.addConstr(BIG_M * z[0][i][k][l++]  >= W + 1 - x[i][k],  "15");

                for(m = 0; m < l; m++)
                    exp += z[0][i][k][m];

                model.addConstr(exp <= (l + 1) /2, "15_1");
            }
        }

        //constr for clbs
        for(i = 0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(clb[i][k] >= x[i][k] - BIG_M * (1 - z[0][i][k][l++]), "8");

                model.addConstr(clb[i][k] >= (x[i][k] - 1)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "9");

                model.addConstr(clb[i][k] >= (x[i][k] - 2)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "10");

                model.addConstr(clb[i][k] >= (x[i][k] - 3)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "11");

                model.addConstr(clb[i][k] >= (x[i][k] - 4)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "12");

                model.addConstr(clb[i][k] >= (x[i][k] - 5)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "13");

                model.addConstr(clb[i][k] >= (x[i][k] - 6)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "14");

                model.addConstr(clb[i][k] >= (x[i][k] - 7)  - BIG_M * (1 - z[0][i][k][l++]) -
                                                           BIG_M * (1 - z[0][i][k][l++]), "15");
            }

            for( k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(x[i][k] >= clb[i][k] - BIG_M * (1 - z[0][i][k][l++]), "16");

                model.addConstr(x[i][k] - 1 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "17");

                model.addConstr(x[i][k] - 2 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "18");

                model.addConstr(x[i][k] - 3 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "19");

                model.addConstr(x[i][k] - 4 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "20");

                model.addConstr(x[i][k] - 5 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "21");

                model.addConstr(x[i][k] - 6 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "22");

                model.addConstr(x[i][k] - 7 >= (clb[i][k])  - BIG_M * (1 - z[0][i][k][l++]) -
                                                       BIG_M * (1 - z[0][i][k][l++]), "23");
            }
        }

        //FBDN region 1
        /******************************************************************
        Constr 2.0.1: The clb on the FPGA is described using the following
                    piecewise function.
                    x       0  <= x < 4
                    x-1     4  <= x < 7
                    x-2     7  <= x < 10
                    x-3     10 <= x < 15
                    x-4     15 <= x < 18
                    x-5     18 <= x < 22
                    x-6     22 <= x < 25
                    x-7     25 <= x < W

                    The piecewise function is then transformed into a set
                    of MILP constraints using the intermediate variable z
        ******************************************************************/
        for(i =0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                GRBLinExpr exp;
                model.addConstr(BIG_M * z[3][i][k][l++]  >= 10 - x[i][k], "1");
                model.addConstr(BIG_M * z[3][i][k][l++]  >= x[i][k] - 9, "2");
                model.addConstr(BIG_M * z[3][i][k][l++]  >= 11 - x[i][k], "3");
                model.addConstr(BIG_M * z[3][i][k][l++]  >= x[i][k] - 10, "4");
                model.addConstr(BIG_M * z[3][i][k][l++]  >= W + 1 - x[i][k], "5");

                for(m = 0; m < l; m++)
                    exp += z[3][i][k][m];

                model.addConstr(exp <= (l + 1) /2, "15_1");
            }
        }

        for(i = 0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;

                //FBDN region
                model.addConstr(clb_fbdn[1][i][k] >= 7 - BIG_M * (1 - z[3][i][k][l++]), "8"); //fs_pynq[1].x - 7

                model.addConstr(clb_fbdn[1][i][k] >= (x[i][k] - 2)  - BIG_M * (1 - z[3][i][k][l++]) -
                                                           BIG_M * (1 - z[3][i][k][l++]), "9");

                model.addConstr(clb_fbdn[1][i][k] >= 8  - BIG_M * (1 - z[3][i][k][l++]) -     //fs_pynq[1].x + fs_pynq[1]. w - 7
                                                           BIG_M * (1 - z[3][i][k][l++]), "10");
            } 
        }

        /******************************************************************
        Constr 2.1: The same thing as constr 1.0.0 is done for the bram
                      which has the following piecewise distribution on
                      the fpga fabric
                    0     0  <=  x  < 4
                    1     4  <=  x  < 18
                    2     18 <=  x  < 25
                    3     25 <=  x  < W
        ******************************************************************/
        for(i =0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                GRBLinExpr exp;
                model.addConstr(BIG_M * z[1][i][k][l++]  >= 4 - x[i][k], "32");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= x[i][k] - 3, "33");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= 18 - x[i][k], "34");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= x[i][k] - 17, "35");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= 25 - x[i][k], "36");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= x[i][k] - 24, "37");
                model.addConstr(BIG_M * z[1][i][k][l++]  >= W + 1 - x[i][k], "38");

                for(m = 0; m < l; m++)
                    exp += z[1][i][k][m];

                model.addConstr(exp <= (l + 1) /2);
            }
        }

//#ifdef bram
        for(i = 0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(bram[i][k] >= 0 - BIG_M * (1 - z[1][i][k][l++]), "39");

                model.addConstr(bram[i][k] >= 1  - BIG_M * (1 - z[1][i][k][l++]) -
                                                           BIG_M * (1 - z[1][i][k][l++]), "40");

                model.addConstr(bram[i][k] >= 2  - BIG_M * (1 - z[1][i][k][l++]) -
                                                           BIG_M * (1 - z[1][i][k][l++]), "41");

                model.addConstr(bram[i][k] >= 3  - BIG_M * (1 - z[1][i][k][l++]) -
                                                           BIG_M * (1 - z[1][i][k][l++]), "42");

            }

            for( k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(0 >= bram[i][k] - BIG_M * (1 - z[1][i][k][l++]), "43");

                model.addConstr(1 >= (bram[i][k])  - BIG_M * (1 - z[1][i][k][l++]) -
                                                       BIG_M * (1 - z[1][i][k][l++]), "44");

                model.addConstr(2 >= (bram[i][k])  - BIG_M * (1 - z[1][i][k][l++]) -
                                                       BIG_M * (1 - z[1][i][k][l++]), "45");

                model.addConstr(3 >= (bram[i][k])  - BIG_M * (1 - z[1][i][k][l++]) -
                                                       BIG_M * (1 - z[1][i][k][l++]), "46");

            }
        }

//#endif

//#ifdef dspp
        /******************************************************************
        Constr 2.2: Same thing is done for the dsp on the FPGA
                    0     0  <=  x  < 7
                    1     7  <=  x  < 22
                    2     22 <=  x  < W
        ******************************************************************/
        for(i =0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                GRBLinExpr exp;
                model.addConstr(BIG_M * z[2][i][k][l++]  >= 7 - x[i][k], "47");
                model.addConstr(BIG_M * z[2][i][k][l++]  >= x[i][k] - 6, "48");
                model.addConstr(BIG_M * z[2][i][k][l++]  >= 22 - x[i][k], "49");
                model.addConstr(BIG_M * z[2][i][k][l++]  >= x[i][k] - 21, "50");
                model.addConstr(BIG_M * z[2][i][k][l++]  >= W + 1 - x[i][k], "51");

                for(m = 0; m < l; m++)
                    exp += z[2][i][k][m];

                model.addConstr(exp <= (l + 1) /2);
           }
        }

        for(i = 0; i < num_slots; i++) {
            for(k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(dsp[i][k] >= (0 - BIG_M * (1 - z[2][i][k][l++])), "52");

                model.addConstr(dsp[i][k] >= (1  - BIG_M * (1 - z[2][i][k][l++]) -
                                                  BIG_M * (1 - z[2][i][k][l++])), "53");

                model.addConstr(dsp[i][k] >= (2  - BIG_M * (1 - z[2][i][k][l++]) -
                                                  BIG_M * (1 - z[2][i][k][l++])), "54");
            }

            for( k = 0; k < 2; k++) {
                l = 0;
                model.addConstr(0 >= dsp[i][k] - BIG_M * (1 - z[2][i][k][l++]), "55");

                model.addConstr(1 >= (dsp[i][k])  - BIG_M * (1 - z[2][i][k][l++]) -
                                                    BIG_M * (1 - z[2][i][k][l++]), "56");

                model.addConstr(2 >= (dsp[i][k])  - BIG_M * (1 - z[2][i][k][l++]) -
                                                    BIG_M * (1 - z[2][i][k][l++]), "57");
            }
       }
//#endif
/*
        for(i = 0; i < num_slots; i++) {
            model.addConstr(BIG_M * res_constr[0][i] >= (bram_req[i] - 9), "333");
            model.addConstr(BIG_M * res_constr[1][i] >= (dsp_req[i]), "334");
            model.addConstr(BIG_M * kappa[1][i] >= res_constr[0][i] + res_constr[1][i] - 1, "335");
            model.addConstr(kappa[1][i] >= 0);
        }*/

      //constr for res
      /*********************************************************************
        Constr 2.3: There must be enough clb, bram and dsp inside the slot
      **********************************************************************/
        for(i = 0; i <  t.maxPartitions /* num_slots */; i++) {
            GRBLinExpr exp_tau, exp_clb, exp_bram, exp_dsp, exp_hw_task;
            GRBLinExpr exp_clb_fbdn;
            for(j = 0; j < num_clk_regs; j++) {
                model.addConstr(tau[0][i][j] <= 10000 * beta[i][j], "58");
                model.addConstr(tau[0][i][j] <= clb[i][1] - clb[i][0], "59");
                model.addConstr(tau[0][i][j] >= (clb[i][1] - clb[i][0]) - (1 - beta[i][j]) * clb_max, "60");
                model.addConstr(tau[0][i][j] >= 0, "15");
               
                //CLB_FBDN 0
                model.addConstr(tau_fbdn[0][0][i][j] <= 10000 * (beta_fbdn[j] + beta[i][j] - 1), "58");
                model.addConstr(tau_fbdn[0][0][i][j] <= clb_fbdn[0][i][1] - clb_fbdn[0][i][0], "59");
                model.addConstr(tau_fbdn[0][0][i][j] >= (clb_fbdn[0][i][1] - clb_fbdn[0][i][0]) - (2 - beta[i][j] - beta_fbdn[j]) * clb_max, "60");
                model.addConstr(tau_fbdn[0][0][i][j] >= 0, "15");

 
                model.addConstr(tau[1][i][j] <= 10000 * beta[i][j], "61");
                model.addConstr(tau[1][i][j] <= bram[i][1] - bram[i][0], "62");
                model.addConstr(tau[1][i][j] >= (bram[i][1] - bram[i][0]) - (1 - beta[i][j]) * bram_max, "63");
                model.addConstr(tau[1][i][j] >= 0, "53");

                model.addConstr(tau[2][i][j] <= 10000 * beta[i][j], "64");
                model.addConstr(tau[2][i][j] <= dsp[i][1] - dsp[i][0], "65");
                model.addConstr(tau[2][i][j] >= (dsp[i][1] - dsp[i][0]) - (1 - beta[i][j]) * dsp_max, "66");
                model.addConstr(tau[2][i][j] >= 0, "67");
                
                exp_clb += tau[0][i][j];
                exp_clb_fbdn  += tau_fbdn[0][0][i][j] + tau_fbdn[1][0][i][j];

                exp_dsp += tau[2][i][j];

                exp_bram += tau[1][i][j];

            }

            //partitioning patch
            for(int a = 0; a < task_set->maxHW_Tasks; a++)
                exp_hw_task += A[a][i];

            //model.addConstr(clb_per_tile * exp_res >= clb_req_zynq[i],"68");
            //model.addConstr(wasted[i][0] == (clb_per_tile * exp_res) - clb_req_zynq[i],"168"); //wasted clbs
            
            //partitioning patch
            model.addConstr(clb_per_tile * (exp_clb - exp_clb_fbdn) >= b[0][i],"68");
            model.addConstr(clb_per_tile * (exp_clb - exp_clb_fbdn) <= BIG_M * exp_hw_task, "con hw1");
            model.addConstr(wasted[i][0] == (clb_per_tile * (exp_clb - exp_clb_fbdn)) - b[0][i],"168"); //wasted clbs

            model.addConstr(clb_fbdn_tot[i] == exp_clb_fbdn, "169");
            //model.addConstr(bram_per_tile * exp_bram >= bram_req_zynq[i],"69");
            //model.addConstr(wasted[i][1] == bram_per_tile * exp_bram - bram_req_zynq[i] ,"169");
            
            //partitioning patch
            model.addConstr(bram_per_tile * exp_bram >= b[1][i],"69");
            model.addConstr(bram_per_tile * exp_bram <= BIG_M * exp_hw_task, "con hw1");
            model.addConstr(wasted[i][1] == bram_per_tile * exp_bram - b[1][i] ,"169");

            //model.addConstr(dsp_per_tile * exp_dsp >= dsp_req_zynq[i],"70");
            //model.addConstr(wasted[i][2] == (dsp_per_tile * exp_dsp) - dsp_req_zynq[i], "170");
            
            //partitioning patch
            model.addConstr(dsp_per_tile * exp_dsp >= b[2][i],"70");
            model.addConstr(dsp_per_tile * exp_dsp <= BIG_M * exp_hw_task, "con hw1");
            model.addConstr(wasted[i][2] == (dsp_per_tile * exp_dsp) - b[2][i], "170");
        }

        //Interference constraints
        /***********************************************************************
        Constraint 3.0: The semantics of Gamma, Alpha, Omega & Psi must be fixed
        ***********************************************************************/
        for(i = 0; i < num_slots; i++) {
            for(k = 0; k < num_slots; k++) {
                if(i == k)
                    continue;
/*              model.addConstr(BIG_M * gamma[i][k] >= x[k][0] - x[i][0], "63");
                model.addConstr(BIG_M * theta[i][k] >= (y[k] - y[i]) * num_rows, "64");
                model.addConstr(BIG_M * Gamma[i][k] >= x[i][1] - x[k][0] + 1, "65");
                model.addConstr(BIG_M * Alpha[i][k] >= x[k][1] - x[i][0] + 1, "66");
                model.addConstr(BIG_M * Omega[i][k] >= y[i] * num_rows + h[i] - y[k] * num_rows, "67");
                model.addConstr(BIG_M * Psi[i][k]   >= y[k] * num_rows + h[k] - y[i] * num_rows, "68");
  */
                model.addConstr(BIG_M * gamma[i][k] >= x[k][0] + 1 - x[i][0], "63");
                model.addConstr(BIG_M * theta[i][k] >= (y[k] - y[i]), "64");
                model.addConstr(BIG_M * Gamma[i][k] >= x[i][1] - x[k][0] + 1, "65");
                model.addConstr(BIG_M * Alpha[i][k] >= x[k][1] - x[i][0] + 1, "66");
                model.addConstr(BIG_M * Omega[i][k] >= y[i] + h[i] - y[k], "67");
                model.addConstr(BIG_M * Psi[i][k]   >= y[k] + h[k] - y[i], "68");
            }
        }

        /***********************************************************************
        Constraint 3.1 Non interference between slot 'i' and 'k'
        ************************************************************************/

        for(i = 0; i < num_slots; i++) {
            GRBLinExpr exp_delta;
            for(k = 0; k < num_slots; k++) {
                if(i == k)
                    continue;
                model.addConstr(delta[0][i][k] >= gamma[i][k] + theta[i][k] +
                                Gamma[i][k] + Omega[i][k] - 3, "69");
                model.addConstr(delta[0][i][k] >= (1- gamma[i][k]) + theta[i][k] +
                                Alpha[i][k] + Omega[i][k] - 3, "70");
                model.addConstr(delta[0][i][k] >= gamma[i][k] + (1 - theta[i][k]) +
                                Gamma[i][k] + Psi[i][k] - 3, "71");
                model.addConstr(delta[0][i][k] >= (1 - gamma[i][k]) + (1 - theta[i][k]) +
                                Alpha[i][k] + Psi[i][k] - 3, "72");
                model.addConstr(delta[0][i][k] == 0, "73");
            }
        }

//#ifdef fbdn
        //Non Interference between global resoureces and slots
        /*************************************************************************
        Constriant 4.0: Global Resources should not be included inside slots
        *************************************************************************/
/*
        for(i = 0; i < num_forbidden_slots; i++) {
            for(k = 0; k < num_slots; k++) {
                model.addConstr(BIG_M * mu[i][k]     >= x[k][0] - fs_zynq[i].x, "74");
                model.addConstr(BIG_M * nu[i][k]     >= y[k] * num_rows - fs_zynq[i].y, "75");
                model.addConstr(BIG_M * fbdn_1[i][k] >= fs_zynq[i].x + fs_zynq[i].w - x[k][0] + 1, "76");
                model.addConstr(BIG_M * fbdn_2[i][k] >= x[k][1]    - fs_zynq[i].x + 1, "77");
                model.addConstr(BIG_M * fbdn_3[i][k] >= fs_zynq[i].y + fs_zynq[i].h - y[k] * num_rows + 1, "78");
                model.addConstr(BIG_M * fbdn_4[i][k] >= y[k] * num_rows + h[k] - fs_zynq[i].y + 1, "79");
            }
        }
*/
        /*************************************************************************
        Constraint 4.1:
        **************************************************************************/
/*
        for(i = 0; i < num_forbidden_slots; i++) {
            //GRBLinExpr exp_delta;

            for(k = 0; k < num_slots; k++) {
                model.addConstr(delta[1][i][k] >= mu[i][k] + nu[i][k] + fbdn_1[i][k] + fbdn_3[i][k] - 3, "80");
                model.addConstr(delta[1][i][k] >= (1- mu[i][k]) + nu[i][k] + fbdn_2[i][k] + fbdn_3[i][k] - 3, "81");
                model.addConstr(delta[1][i][k] >= mu[i][k] + (1 - nu[i][k]) + fbdn_1[i][k] + fbdn_4[i][k] - 3, "82");
                model.addConstr(delta[1][i][k] >= (1 - mu[i][k]) + (1 - nu[i][k]) + fbdn_2[i][k] + fbdn_4[i][k] - 3, "83");

                //exp_delta += delta[1][i][k];
                model.addConstr(delta[1][i][k] == 0, "84");
            }
        }
*/
//#endif

        /*************************************************************************
        Constraint 4.2:
        **************************************************************************/

        for(i = 0; i < num_slots; i++) {
            for (j = 0; j <  num_fbdn_edge; j++) {
                l = 0;
                model.addConstr(x[i][0] - forbidden_boundaries_left[j] <= -0.01 + kappa[i][j][l] * BIG_M, "edge_con");
                model.addConstr(x[i][0] - forbidden_boundaries_left[j] >= 0.01 - (1 - kappa[i][j][l]) * BIG_M, "edge_con_1");
                l++;
                model.addConstr(x[i][1] - forbidden_boundaries_right[j] <= -0.01 + kappa[i][j][l] * BIG_M, "edge_con_2");
                model.addConstr(x[i][1] - forbidden_boundaries_right[j] >= 0.01 - (1 - kappa[i][j][l]) * BIG_M, "edge_con_3");
            }
        }

//#ifdef objective
        //Objective function parameters definition
        /*************************************************************************
        Constriant 5.0: The centroids of each slot and the distance between each
                        of them (the wirelength) is defined in these constraints.
                        The wirelength is used in the objective function
        *************************************************************************/
        GRBLinExpr obj_x, obj_y, obj_wasted_clb, obj_wasted_bram, obj_wasted_dsp;
        unsigned long wl_max = 0;

        for(i = 0; i < num_slots; i++) {
            model.addConstr(centroid[i][0] == x[i][0] + w[i] / 2, "84");
            model.addConstr(centroid[i][1] == y[i] * 10 + h[i] * 10 / 2, "86");
        }

        for(i =0; i < num_slots; i++){
            for(j = 0; j < num_slots; j++) {
                if(i >= j ) {
                    continue;
                }
                model.addConstr(dist[i][j][0] >= (centroid[i][0] - centroid[j][0]), "87");
                model.addConstr(dist[i][j][0] >= (centroid[j][0] - centroid[i][0]), "88");
                model.addConstr(dist[i][j][1] >= (centroid[i][1] - centroid[j][1]), "89");
                model.addConstr(dist[i][j][1] >= (centroid[j][1] - centroid[i][1]), "90");
            }
        }


        for(i = 0; i < num_slots; i++) {
/*            for(j = 0; j < num_slots; j++) {
                if(i >= j)
                    continue;
                obj_x += dist[i][j][0];
                obj_y += dist[i][j][1];
            }
*/
            obj_wasted_clb  += wasted[i][0];
            obj_wasted_bram += wasted[i][1];
            obj_wasted_dsp  += wasted[i][2];
        }


        cout <<"added opt" <<endl;

        if(num_conn_slots_zynq > 0) {
        for(i = 0; i < num_conn_slots_zynq; i++) {
                dist_0 = conn_matrix_zynq[i][0] - 1;
                dist_1 = conn_matrix_zynq[i][1] - 1;
                dist_2 = conn_matrix_zynq[i][2];

                obj_x += dist[dist_0] [dist_1][0] * dist_2;
                obj_y += dist[dist_0] [dist_1][1] * dist_2;
        }


        for(i = 0; i < num_conn_slots_zynq; i++) {
            wl_max += conn_matrix_zynq[i][2] * (W + H * 20);
        }

        cout<< "W H and wl max is " << W << " " << H * 20 << " "<< wl_max <<endl;
}

        //model.setObjective((obj_x + obj_y), GRB_MINIMIZE);
        //model.setObjective(1 * obj_wasted_clb,  GRB_MINIMIZE);
        ////model.setObjective(obj_wasted_bram, GRB_MINIMIZE);
        model.setObjective(obj_wasted_clb,  GRB_MINIMIZE);
//#endif

        //Optimize
        /****************************************************************************
        Optimize
        *****************************************************************************/
        model.set(GRB_IntParam_Threads, 8);
        model.set(GRB_DoubleParam_TimeLimit, 120);
        model.optimize();
        wasted_clb_zynq = 0;
        wasted_bram_zynq  = 0;
        wasted_dsp_zynq  = 0;
        unsigned long w_x = 0, w_y = 0;

        status = model.get(GRB_IntAttr_Status);

        if(status == GRB_OPTIMAL) {
            // ------------------------------------------------------------
            // Partition OUTPUT
            // ------------------------------------------------------------
            cout << "--------------------------------------------------------------------" << endl;
            cout << "-) HW-TASK ALLOCATION" << endl;
            cout << "--------------------------------------------------------------------" << endl;
            cout << endl;
            cout << "Partition: \t";
                for(uint k=0; k < t.maxPartitions; k++)
                    cout << k << "\t";
                cout << endl;
            cout << "--------------------------------------------------------------------" << endl;
            for(uint a=0; a < t.maxHW_Tasks; a++)
            {
                cout << "HW-Task [" << a << "] : \t";
                for(uint k=0; k < t.maxPartitions; k++)
                {
                    if((unsigned int)A[a][k].get(GRB_DoubleAttr_X))
                        cout << "X" << "\t";
                    else
                        cout << " " << "\t";
                    //cout << A[a][k].get(GRB_DoubleAttr_X) << "\t";
                }

                cout << endl;
            }
//#ifdef dspppp
            cout << endl;
            cout << "--------------------------------------------------------------------" << endl;
            cout << "-) RECONFIGURATION TIMES" << endl;
            cout << "--------------------------------------------------------------------" << endl;
            cout << endl;

            cout << endl;

            /*for(uint a=0; a < t.maxHW_Tasks; a++)
                for(uint b=0; b < t.maxHW_Tasks; b++)
                    cout << "I_SLOT["<<a<<"]["<<b<<"] = " << I_SLOT[a][b].get(GRB_DoubleAttr_X)<<endl;*/

            for(uint a=0; a < t.maxHW_Tasks; a++)
                cout << "r["<<a<<"] = " << r[a].get(GRB_DoubleAttr_X)<<endl;

            for(uint a=0; a < t.maxHW_Tasks; a++){
                for(uint k=0; k < t.maxPartitions; k++)
                    if((unsigned int)A[a][k].get(GRB_DoubleAttr_X))
                        cout << "r["<<a<<"]["<<k<<"] = CLB " << b[0][a].get(GRB_DoubleAttr_X)<< " Bram " <<b[1][a].get(GRB_DoubleAttr_X)
                             <<" DSP " <<b[2][a].get(GRB_DoubleAttr_X)<<endl;
            }

            for(uint a=0; a < t.maxHW_Tasks; a++)
                cout << "gamma_part["<<a<<"] = " << gamma_part[a].get(GRB_DoubleAttr_X)<<endl;

//#endif
            //stores the chosen partitions
            vector <unsigned long> active_partitions (t.maxPartitions, 0);

            for(uint k=0; k < t.maxPartitions; k++)
                for(uint a=0; a < t.maxHW_Tasks; a++)
                    if((unsigned int)A[a][k].get(GRB_DoubleAttr_X))
                        active_partitions[k] = 1;

            for(uint k=0; k < t.maxPartitions; k++)
                num_active_partitions += active_partitions[k];

            cout<< "num partitions " <<num_active_partitions <<endl;

            // ------------------------------------------------------------
            // packing allocation to struct
            // ------------------------------------------------------------
            bool is_part_allocated = false;
            unsigned int index = 0, num_tasks_in_part, task_id_in_part;

            for(uint k = 0; k < t.maxPartitions; k++) {
                is_part_allocated = false;
                num_tasks_in_part = 0;

                for(uint a = 0; a < t.maxHW_Tasks; a++){
                    if((unsigned int)A[a][k].get(GRB_DoubleAttr_X)) {
                        is_part_allocated = true;
                        num_tasks_in_part += 1;
                        (*to_sim->task_alloc)[index].task_id.push_back(a);
                    }
                }
                if(is_part_allocated) {
                    (*to_sim->task_alloc)[index].num_tasks_in_part = num_tasks_in_part;
                    index += 1;
                }
            }

            //Get which partitions has the highest number of RMs
            unsigned long max_modules_per_partition = 0;
            for(uint k = 0; k < num_active_partitions; k++) {
                if ((*to_sim->task_alloc)[k].num_tasks_in_part > max_modules_per_partition)
                    max_modules_per_partition = (*to_sim->task_alloc)[k].num_tasks_in_part;
            }

            to_sim->max_modules_per_partition = max_modules_per_partition;

            cout << endl;
            cout << "--------------------------------------------------------------------" << endl;
            cout << " RESOURCE REQUIREMENT OF RMs" << endl;
            cout << "--------------------------------------------------------------------" << endl;
            cout << endl;

            for(i = 0; i < num_slots; i++) {
                cout << "RM_" << i <<"\t" <<"CLB = " << clb_req_zynq[i] << endl;
                cout << "\t" << "BRAM = " << bram_req_zynq[i] << endl;
                cout << "\t" << "DSP  = " << dsp_req_zynq[i] << endl;

                cout <<endl;
            }

 
            to_sim->num_partition = num_active_partitions;
            cout <<endl;

            for(uint k = 0, m = 0; k < t.maxPartitions; k++) {
                if(active_partitions[k] == 1) { 
                    for (uint a = 0; a < num_slots; a++) { 
                        if(A[a][k].get(GRB_DoubleAttr_X) == 1) {
                            if(task_set->HW_Tasks[a].resDemand[CLB] > clb_in_partition[m]) 
                               clb_in_partition[m] = task_set->HW_Tasks[a].resDemand[CLB];

                             if(task_set->HW_Tasks[a].resDemand[BRAM] > bram_in_partition[m])
                                    bram_in_partition[m] = task_set->HW_Tasks[a].resDemand[BRAM];
                              
                              if(task_set->HW_Tasks[a].resDemand[DSP] > dsp_in_partition[m])
                                    dsp_in_partition[m] = task_set->HW_Tasks[a].resDemand[DSP];            
                        }
                    }
                    m += 1;
                }
            }

            cout << "---------------------------------------------------------------------\
----------------------------------------------------------------------------------------------------\
------------------------  "<< endl;
            cout<< "slot \t" << "x_0 \t" << "x_1 \t" << "y \t" << "w \t" << "h \t" << "clb_0 \t"
                    << "clb_1 \t" << "clb \t" << "req\t" << "bram_0 \t" << "bram_1 \t" << "bram \t"
                    << "req\t" << "dsp_0 \t" << "dsp_1 \t" << "dsp\t" << "req" <<endl;
            cout << "----------------------------------------------------------------------\
--------------------------------------------------------------------------------------------\
-------------------------------" << endl;
    
            for(i = 0, m = 0; i < num_slots; i++) {
                if(active_partitions[i]) {
                    (*to_sim->x)[m] = (int) x[i][0].get(GRB_DoubleAttr_X);
		            (*to_sim->y)[m] = (int) y[i].get(GRB_DoubleAttr_X) * 10;
		            (*to_sim->w)[m] = (int) w[i].get(GRB_DoubleAttr_X);
                    (*to_sim->h)[m] = (int) h[i].get(GRB_DoubleAttr_X) * 10;
                    (*to_sim->clb_from_solver)[m] =  (int) (((clb[i][1].get(GRB_DoubleAttr_X) - 
                                                     clb[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) -
                                                     clb_fbdn_tot[i].get(GRB_DoubleAttr_X)) * clb_per_tile);

                    (*to_sim->bram_from_solver)[m] = (int) (((bram[i][1].get(GRB_DoubleAttr_X) - 
                                                    bram[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) -
                                                    bram_fbdn_tot[i].get(GRB_DoubleAttr_X)) * bram_per_tile);

                    (*to_sim->dsp_from_solver)[m] =  (int) (((dsp[i][1].get(GRB_DoubleAttr_X) - 
                                                     dsp[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) -
                                                     dsp_fbdn_tot[i].get(GRB_DoubleAttr_X)) * dsp_per_tile);
 //               }


                    cout << m << "\t" << x[i][0].get(GRB_DoubleAttr_X) <<"\t"
                    << x[i][1].get(GRB_DoubleAttr_X) << "\t" << y[i].get(GRB_DoubleAttr_X)
                    <<" \t" <<  w[i].get(GRB_DoubleAttr_X) << "\t" << h[i].get(GRB_DoubleAttr_X)

                    <<"\t" << clb[i][0].get(GRB_DoubleAttr_X) <<"\t" <<
                    clb[i][1].get(GRB_DoubleAttr_X) << "\t" << ((clb[i][1].get(GRB_DoubleAttr_X) -
                     clb[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) - clb_fbdn_tot[i].get(GRB_DoubleAttr_X)) *
                     clb_per_tile << "\t" << clb_in_partition[m]

                    <<"\t" << bram[i][0].get(GRB_DoubleAttr_X) <<"\t" <<
                    bram[i][1].get(GRB_DoubleAttr_X) << "\t" << ((bram[i][1].get(GRB_DoubleAttr_X) -
                    bram[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) - bram_fbdn_tot[i].get(GRB_DoubleAttr_X)) *
                    bram_per_tile << "\t" << bram_in_partition[m]

                    << "\t" << dsp[i][0].get(GRB_DoubleAttr_X) << "\t" <<
                    dsp[i][1].get(GRB_DoubleAttr_X) << "\t" << ((dsp[i][1].get(GRB_DoubleAttr_X) -
                            dsp[i][0].get(GRB_DoubleAttr_X)) * h[i].get(GRB_DoubleAttr_X) - dsp_fbdn_tot[i].get(GRB_DoubleAttr_X)) *
                             dsp_per_tile << "\t" << dsp_in_partition[m] <<endl;
                    
                    m += 1;
                }
            }
//              cout <<endl;
/*
                cout << "num clbs in forbidden slot is " << clb_fbdn_tot[i].get(GRB_DoubleAttr_X) * clb_per_tile <<endl;
                cout << "num clb 0 in forbidden slot "<< 0 <<" is " << clb_fbdn[0][i][0].get(GRB_DoubleAttr_X) <<endl;
                cout << "num clbs 1 in forbidden slot " <<0 << "is " << clb_fbdn[0][i][1].get(GRB_DoubleAttr_X) <<endl;
                
                cout <<endl; 
                cout << "num clb 0 in forbidden slot " << 1 << " is " << clb_fbdn[1][i][0].get(GRB_DoubleAttr_X) <<endl;
                cout << "num clbs 1 in forbidden slot " << 1 << " is " << clb_fbdn[1][i][1].get(GRB_DoubleAttr_X) <<endl;
                
                cout << endl;
                cout << "num bram in forbidden slot is " << bram_fbdn_tot[i].get(GRB_DoubleAttr_X) * bram_per_tile <<endl;
                cout << "num bram 0 in forbidden slot "<< i <<" is " << bram_fbdn[0][i][0].get(GRB_DoubleAttr_X) <<endl;
                cout << "num bram 1 in forbidden slot " <<i << "is " << bram_fbdn[0][i][1].get(GRB_DoubleAttr_X) <<endl;
                
                cout <<endl;
                cout << "total dsp in forbidden slot is " << dsp_fbdn_tot[i].get(GRB_DoubleAttr_X) * dsp_per_tile <<endl;
                cout << "num dsp 0 in forbidden slot " << i << " is " << dsp_fbdn[1][i][0].get(GRB_DoubleAttr_X) <<endl;
                cout << "num dsp 1 in forbidden slot " << i << " is " << dsp_fbdn[1][i][1].get(GRB_DoubleAttr_X) <<endl;

*/

/*
                for(k=0; k < 2; k++) {
                    for(l = 0; l < 14; l++)
                         cout <<"z" << l << " " << z[3][i][k][l].get(GRB_DoubleAttr_X) << "\t";
                         cout <<endl;
                }

                for(k=0; k < 2; k++) {
                    for(l = 0; l < 9; l++)
                        cout <<"z" << l << " " << z[3][i][k][l].get(GRB_DoubleAttr_X) << "\t";
                        cout<<endl;
                }

                for(k=0; k < 2; k++) {
                    for(l = 0; l < 4; l++)
                        cout <<"z" << l << " " << z[3][i][k][l].get(GRB_DoubleAttr_X) << "\t";
                        cout<<endl;
                }
*/

            


            for(uint k = 0; k < t.maxPartitions; k++) {
                if(active_partitions[k] == 1) {
/*
            for (i = 0; i < num_slots; i++) {
                wasted_clb_zynq  +=  wasted[i][0].get(GRB_DoubleAttr_X);
                wasted_bram_zynq +=  wasted[i][1].get(GRB_DoubleAttr_X);
                wasted_dsp_zynq  +=  wasted[i][2].get(GRB_DoubleAttr_X);
*/
                cout << "clb req is " << b[0][k].get(GRB_DoubleAttr_X) 
                     << " bram is " << b[1][k].get(GRB_DoubleAttr_X) 
                     << " dsp is " << b[2][k].get(GRB_DoubleAttr_X) <<endl;

                cout << "wasted clb " << wasted[k][0].get(GRB_DoubleAttr_X) << " wasted bram " << wasted[k][1].get(GRB_DoubleAttr_X) <<
                       " wasted dsp " << wasted[k][2].get(GRB_DoubleAttr_X) <<endl;
                
                cout << "max " << k;
                for (i = 0; i < t.maxHW_Tasks; i++)
                    cout << " " << max[i][k].get(GRB_DoubleAttr_X); 
                
                cout <<endl;
                }
            }

/*
                cout<< "centroid " << i << centroid[i][0].get(GRB_DoubleAttr_X) << " " <<centroid[i][1].get(GRB_DoubleAttr_X) <<endl;
                for(j = 0; j < num_slots; j++) {
                    if(i >= j)
                        continue;
                    w_x += dist[i][j][0].get(GRB_DoubleAttr_X);
                    w_y += dist[i][j][1].get(GRB_DoubleAttr_X);

                }

            }

                cout << "total wasted clb " <<wasted_clb_zynq << " total wasted bram " <<
                        wasted_bram_zynq << " total wastd dsp " << wasted_dsp_zynq <<endl;
                cout << " total wire length " << w_x << "  " << w_y << "  " <<  w_y + w_x << endl;
         }
*/
  
  }
  else {

                model.set(GRB_IntParam_Threads, 8);
                model.set(GRB_DoubleParam_TimeLimit, 120);
                model.computeIIS();

            cout<< "the following constraints can not be satisfied" <<endl;
            c = model.getConstrs();

            for(i = 0; i < model.get(GRB_IntAttr_NumConstrs); i++)
                if(c[i].get(GRB_IntAttr_IISConstr) == 1)
                    cout << c[i].get(GRB_StringAttr_ConstrName) << endl;
        }

    /*else {
       model.computeIIS();

        cout<< "the following constraints can not be satisfied" <<endl;
        c = model.getConstrs();

        for(i = 0; i < model.get(GRB_IntAttr_NumConstrs); i++)
            if(c[i].get(GRB_IntAttr_IISConstr) == 1)
                cout << c[i].get(GRB_StringAttr_ConstrName) << endl;

    } */
}
    catch(GRBException e)
    {
        cout << "Error code =" << e.getErrorCode() << endl;
        cout<< e.getMessage() << endl;
        exit(EXIT_FAILURE);
    }
    catch (...)
    {
        cout <<"exception while solving milp" << endl;
        exit(EXIT_FAILURE);
    }

   return status;
}

int zynq_start_optimizer(param_to_solver *param, param_from_solver *to_sim)
{ 
    int m = 0;
    int k = 0;
    int temp;
    unsigned long i;

    num_slots = param->num_rm_modules;
    num_forbidden_slots = param->num_forbidden_slots;
    num_rows = param->num_rows;
    H = param->num_clk_regs;
    W = param->width;

    num_clk_regs = param->num_clk_regs;
    num_conn_slots_zynq = (param->num_connected_slots);
    clb_per_tile = param->clb_per_tile;
    bram_per_tile = param->bram_per_tile;
    dsp_per_tile = param->dsp_per_tile;

    task_set = param->task_set;
    platform = param->platform;
    slacks =   *param->slacks;

    for(i = 0; i < num_slots; i++) {
        clb_req_zynq[i]  = (*param->clb)[i];
        bram_req_zynq[i] = (*param->bram)[i];
        dsp_req_zynq[i]  = (*param->dsp)[i];

        //cout << "clb " << clb_req_zynq[i] << " bram " << 
        //bram_req_zynq[i] << "dsp " << dsp_req_zynq[i] << endl;
    }

    for(i = 0; i < task_set->maxPartitions; i++) {
        clb_in_partition[i] = 0;
        bram_in_partition[i] = 0;
        dsp_in_partition[i] = 0;
    }

    for(i = 0; i < num_conn_slots_zynq; i++) {
        for(k = 0; k < 3; k++)
            conn_matrix_zynq[i][k] = (*(param->conn_vector)) [i][k];
    }

    m =0;
    for(i = 0; i < num_conn_slots_zynq; i++) {
        m =0;
 //       for(k = 0; k < 3; k++)
        temp = conn_matrix_zynq[i][m++] + conn_matrix_zynq[i][m++] + conn_matrix_zynq[i][m++];
            //cout << "inside solver " << temp << /*m << conn_matrix[i][m++] << " m " << m << conn_matrix[i][m++] << " m" << m << << conn_matrix[i][k] <<*/endl;
            //k++;
            //cout << "k is " << k /*conn_matrix[i][k++] */<< endl;
            //k++;
            // cout << conn_matrix[i][k] << endl;
    }
    for(i = 0; i < num_forbidden_slots; i++) {
        fs_zynq[i] = (*param->fbdn_slot)[i];
        //cout <<"forbidden " << num_forbidden_slots << " " << fs_zynq[i].x << " " << fs_zynq[i].y << " " << fs_zynq[i].h << " " << fs_zynq[i].w <<endl;
    }

    cout << "ZYNQ_OPT: starting ZYNQ optimizer" << endl;
    status = solve_milp(*task_set, *platform, slacks, false, to_sim); 
    return 0;
}
