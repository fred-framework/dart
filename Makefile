CC = g++
CFLAGS = -Iwithout_gui/include/
CFLAGS += -Iinclude/
CFLAGS += -std=c++17
CFLAGS += -L$(GUROBI_HOME)/lib
CFLAGS += -ggdb -g3

ifndef GUROBI_HOME
$(error GUROBI_HOME is not defined)
endif

ifndef DART_HOME
$(error DART_HOME is not defined)
endif


LDFLAGS = -lgurobi_g++5.2 -lgurobi_c++ -lgurobi81 -lm -lstdc++fs

all:
	@echo "Please run the make file using the following format"
	@echo ""
	@echo "make target FPGA=type_of_FPGA"
	@echo ""
	@echo "please use a specific target "
	@echo "'flora_with_partitioning'  ---> floorplanner with partitioning"
	@echo "'flora_without_partitioning' ---> only floorplanner without partitioning"
	@echo "'pr_tool_with_part' ---> run the PR flow with including floorplanning and partitoning'" 
	@echo "'pr_tool_without_part' ---> run the PR flow including only the floorplanning "
	@echo " "	
	@echo "for type of FPGA please use ZYNQ or PYNQ"
	@echo " "
	@echo "For example make pr_tool_with_part FPGA=PYNQ"


SOURCES_SHARED = src/csv_data_manipulator.cpp include/fpga.h without_gui/src/main.cpp

ifeq ($(FPGA),PYNQ)
SOURCES_SHARED += include/pynq.h src/pynq.cpp include/pynq_fine_grained.h src/pynq_fine_grained.cpp
CFLAGS += -DFPGA_PYNQ
else ifeq ($(FPGA),ZYNQ)
SOURCES_SHARED += include/zynq.h src/zynq.cpp include/zynq_fine_grained.h src/zynq_fine_grained.cpp
CFLAGS += -DFPGA_ZYNQ
else 
SOURCES_SHARED += include/zynq.h src/zynq.cpp include/zynq_fine_grained.h src/zynq_fine_grained.cpp
CFLAGS += -DFPGA_ZYNQ
endif


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
pr_tool_without_part: BIN = run_pr_tool_without_part
pr_tool_without_part: build

pr_tool_with_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_with_part: SOURCES += src/partition.cpp
pr_tool_with_part: SOURCES += without_gui/src/flora.cpp
pr_tool_with_part: CFLAGS  += -DWITH_PARTITIONING
pr_tool_with_part: BIN = run_pr_tool_with_part
pr_tool_with_part: build

build:
	$(CC) -o bin/$(BIN) $(CFLAGS) $(SOURCES_SHARED) $(SOURCES_MILP) $(SOURCES) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f bin/*run*	
