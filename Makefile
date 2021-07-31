CC = g++
CFLAGS = -Iwithout_gui/include/
CFLAGS += -Iinclude/
CFLAGS += -std=c++17
CFLAGS += -L$(GUROBI_HOME)/lib
# eliminate some warnings
CFLAGS += -Wno-unused-result
# change here to switch from debug to optimized compilation mode
CFLAGS += -O2
#CFLAGS += -ggdb -g3

ifndef GUROBI_HOME
$(error GUROBI_HOME is not defined)
endif

ifndef DART_HOME
$(error DART_HOME is not defined)
endif

# gurobi version MUST be 8.1. It does not work with version 9.1
LDFLAGS = -lgurobi_g++5.2 -lgurobi_c++ -lgurobi81 -lm -lstdc++fs

help:
	@echo "Please run the make file using the following format"
	@echo ""
	@echo "make target FPGA=type_of_FPGA"
	@echo ""
	@echo "please use a specific target "
	@echo "'flora_with_partitioning'  ---> floorplanner with partitioning"
	@echo "'flora_without_partitioning' ---> only floorplanner without partitioning"
	@echo "'pr_tool_with_part' ---> run the PR flow with including floorplanning and partitioning" 
	@echo "'pr_tool_without_part' ---> run the PR flow including only the floorplanning"
	@echo "'pr_all' ---> compile both PR flow executables: floorplanning and partitioning; floorplanning only"
	@echo "'flora_all' ---> compile both floorplanner executables: floorplanning and partitioning; floorplanning only"
	@echo "'all_all' ---> compile the four executables: two for flora and two for the PR flow"
	@echo " "	
	@echo "for type of FPGA please use ZYNQ or PYNQ"
	@echo " "
	@echo "For example make pr_tool_with_part FPGA=PYNQ"


.PHONY: pr_all flora_all all_all

pr_all: 
	make pr_tool_with_part 
	make pr_tool_without_part 

flora_all:
	make flora_with_partitioning 
	make flora_without_partitioning 

all_all: pr_all flora_all

SOURCES_SHARED = src/csv_data_manipulator.cpp include/fpga.h without_gui/src/main.cpp

ifeq ($(FPGA),PYNQ)
SOURCES_SHARED += include/pynq.h src/pynq.cpp include/pynq_fine_grained.h src/pynq_fine_grained.cpp
CFLAGS += -DFPGA_PYNQ
else ifeq ($(FPGA),ZYNQ)
SOURCES_SHARED += include/zynq.h src/zynq.cpp include/zynq_fine_grained.h src/zynq_fine_grained.cpp
CFLAGS += -DFPGA_ZYNQ
# remove the default value. this way LOWER_FPGA will work
#else 
#SOURCES_SHARED += include/zynq.h src/zynq.cpp include/zynq_fine_grained.h src/zynq_fine_grained.cpp
#CFLAGS += -DFPGA_ZYNQ
else ifeq ($(FPGA),US)
SOURCES_SHARED += include/ultrascale.h src/us.cpp include/ultrascale_fine_grained.h src/us_fine_grained.cpp
CFLAGS += -DFPGA_US
else ifeq ($(FPGA),US_96)
SOURCES_SHARED += include/ultrascale_96.h src/us_96.cpp include/ultrascale_96_fine_grained.h src/us_96_fine_grained.cpp
CFLAGS += -DFPGA_US_96
endif

LOWER_FPGA  = $(shell echo $(FPGA) | tr A-Z a-z)

ifeq ($(FPGA),PYNQ)
flora_with_partitioning: SOURCES_MILP = src/milp_model_pynq_with_partition.cpp
flora_without_partitioning: SOURCES_MILP = src/milp_model_pynq.cpp
pr_tool_without_part: SOURCES_MILP = src/milp_model_pynq.cpp
pr_tool_with_part: SOURCES_MILP = src/milp_model_pynq_with_partition.cpp
else ifeq ($(FPGA),ZYNQ)
flora_with_partitioning: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
flora_without_partitioning: SOURCES_MILP = src/milp_model_zynq.cpp
pr_tool_without_part: SOURCES_MILP = src/milp_model_zynq.cpp
pr_tool_with_part: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
else ifeq ($(FPGA),US)
#flora_with_partitioning: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
flora_without_partitioning: SOURCES_MILP = src/milp_model_us.cpp
pr_tool_without_part: SOURCES_MILP = src/milp_model_us.cpp
#pr_tool_with_part: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
else ifeq ($(FPGA),US_96)
#flora_with_partitioning: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
flora_without_partitioning: SOURCES_MILP = src/milp_model_us_96.cpp
pr_tool_without_part: SOURCES_MILP = src/milp_model_us_96.cpp
#pr_tool_with_part: SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
endif

flora_with_partitioning: SOURCES += src/partition.cpp
flora_with_partitioning: SOURCES += without_gui/src/flora.cpp
flora_with_partitioning: BIN = run_with_partition
flora_with_partitioning: CFLAGS += -DWITH_PARTITIONING -DRUN_FLORA
flora_with_partitioning: build


flora_without_partitioning: SOURCES += without_gui/src/flora.cpp
flora_without_partitioning: BIN = run_without_partition
flora_without_partitioning: CFLAGS += -DRUN_FLORA
flora_without_partitioning: build


pr_tool_without_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_without_part: SOURCES += without_gui/src/flora.cpp
pr_tool_without_part: CFLAGS += -DWITHOUT_PARTITIONING
pr_tool_without_part: BIN = run_pr_tool_without_part
pr_tool_without_part: build

pr_tool_with_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_with_part: SOURCES += src/partition.cpp
pr_tool_with_part: SOURCES += without_gui/src/flora.cpp
pr_tool_with_part: CFLAGS  += -DWITH_PARTITIONING
pr_tool_with_part: BIN = run_pr_tool_with_part
pr_tool_with_part: build

build:
	$(CC) -o bin/$(BIN)_$(LOWER_FPGA) $(CFLAGS) $(SOURCES_SHARED) $(SOURCES_MILP) $(SOURCES) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f bin/*run*	
