CC = g++
CFLAGS = -Iwithout_gui/include/
CFLAGS += -Iinclude/
CFLAGS += -std=c++17

LDFLAGS = -lgurobi_g++5.2 -lgurobi_c++ -lgurobi81 -lm -lstdc++fs

all:
	@echo "please use a target 'with_partioning' or 'without_partitioning'" 
	@echo "For example without_partitioning ENABLE_GUI=0"

SOURCES_SHARED = src/csv_data_manipulator.cpp src/fpga.cpp src/fine_grained.cpp
SOURCES_SHARED += without_gui/src/main.cpp

with_partitioning: SOURCES += src/partition.cpp src/milp_model_zynq_with_partition.cpp
with_partitioning: SOURCES += without_gui/src/flora.cpp
with_partitioning: BIN = run_with_partition
with_partitioning: CFLAGS += -DWITH_PARTITIONING -DRUN_FLORA
with_partitioning: build

without_partitioning: SOURCES += src/milp_model_zynq.cpp without_gui/src/flora.cpp
without_partitioning: BIN = run_without_partition
without_partitioning: CFLAGS += -DRUN_FLORA
without_partitioning: build

pr_tool_without_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_without_part: SOURCES += src/partition.cpp src/milp_model_zynq.cpp
pr_tool_without_part: SOURCES += without_gui/src/flora.cpp
#pr_tool_without_part: without_partitioning
pr_tool_without_part: BIN = run_pr_tool_without_part
pr_tool_without_part: build


pr_tool_with_part: SOURCES = without_gui/src/pr_tool.cpp
pr_tool_with_part: SOURCES += src/partition.cpp src/milp_model_zynq_with_partition.cpp
pr_tool_with_part: SOURCES += without_gui/src/flora.cpp
pr_tool_with_part: CFLAGS  += -DWITH_PARTITIONING
pr_tool_with_part: BIN = run_pr_tool_with_part
pr_tool_with_part: build

build:
	$(CC) -o $(BIN) $(CFLAGS) $(SOURCES_SHARED) $(SOURCES) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f *run*	
