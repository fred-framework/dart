CC = g++
CFLAGS = -Iwithout_gui/include/
CFLAGS += -Iinclude/
CFLAGS += -std=c++17

LDFLAGS = -lgurobi_g++5.2 -lgurobi_c++ -lgurobi81 -lm -lstdc++fs

all:
	@echo "please use a target 'pr_tool_with_part' or 'pr_tool_without_partitioning'" 
	@echo "For example make pr_tool_with_part ENABLE_GUI=0 FPGA=PYNQ"

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
SOURCES_MILP = src/milp_model_pynq_with_partition.cpp
else ifeq($(FPGA),ZYNQ)
SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
else
SOURCES_MILP = src/milp_model_zynq_with_partition.cpp
endif

with_partitioning: SOURCES += src/partition.cpp  
with_partitioning: SOURCES += without_gui/src/flora.cpp
with_partitioning: BIN = run_with_partition
with_partitioning: CFLAGS += -DWITH_PARTITIONING -DRUN_FLORA
with_partitioning: build

without_partitioning: SOURCES += src/milp_model_zynq.cpp 
without_partitioning: SOURCES += without_gui/src/flora.cpp
without_partitioning: BIN = run_without_partition
without_partitioning: CFLAGS += -DRUN_FLORA
without_partitioning: build

pr_tool_without_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_without_part: SOURCES += src/partition.cpp
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
