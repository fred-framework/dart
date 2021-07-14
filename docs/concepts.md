

## Main Concepts

### FPGA Offloading Programming Model

In a typical co-design flow, for implementing real-time systems on a DPR-enabled FPGA-based SoC platform, the ensemble of tasks of the system are first classified into (*i*) software tasks, and (*ii*) hardware tasks. Software tasks are regular software activities executed on the CPUs available on the SoC, while hardware tasks are hardware description language (HDL) implementations of computationally-intensive functions to be offloaded on the FPGA. The next figure depicts a sample of a programming model supported by DART, where a software task invokes two hardware acceleration requests at different times. After each invocation the software task self-suspends until the acceleration request is finished. Upon receiving each acceleration request, the reconfiguration interface loads the bitstreams onto the FPGA and the accelerator starts to execute. At the completion of the acceleration, the software task is notified. 

![DART programming model](docs/images/dart1.png)

### Simplified Design Flow

At the beginning of the design flow, the hardware tasks are logically partitioned and assigned to one or more **reconfigurable regions (RRs)** defined in the total FPGA area. Each RR can host more than one hardware module, which will be executed in a time-multiplexed manner. DART adopts a partitioning based on the timing behavior of both software and hardware tasks of the system. 
The timing requirements are mapped into timing-related constraints to ensure that the partitioning does not violate the timing requirements of the software and hardware tasks.

The next Figure represents a block diagram of DART's internal organization, including its inputs and outputs. DART takes as inputs: (*i*) the HDL sources of hardware tasks; (*ii*) the timing requirements of software tasks; and (*iii*) the description of the FPGA internal architecture. DART's outputs include: (*i*) the bitstreams of the design; (*ii*) the necessary files for the [FRED](http://fred.santannapisa.it/runtime/) runtime environment. Inside DART, the partitioning and floorplanning are combined into a single mixed integer linear programming (MILP)-based multi-objective optimization problem. The two DPR design steps were fused into a single optimization problem by converting the input timing requirements and floorplanning related constraints into a series of MILP constraints. For instance, if two hardware tasks are partitioned on the same RR, then the RR must contain enough FPGA resources to host both of them in mutual exclusion, hence requiring the maximum of the FPGA resources requested by each hardware task for each resource type (CLB, DRAM, DSP). At the same time, the two hardware tasks must be capable of tolerating the delays that can be originated due to contention of the RR, i.e., in the worst case, one of the two must wait for the entire time the other occupies the RR before being able to be dynamically configured on it.

![DART design flow](docs/images/dart2.png)

### DART Partition

### Reconfigurable Region

### Slot ?!?!?

## DART Modes

### Without partitioning



### With partitioning
