CC = g++
CFLAGS = -Iwithout_gui/include/
CFLAGS += -Iinclude/
CFLAGS += -std=c++11

LDFLAGS = -lgurobi_g++5.2 -lgurobi_c++ -lgurobi81 -lm

all:
	@echo "please use a target 'with_partioning' or 'without_partitioning'" 
	@echo "For example without_partitioning ENABLE_GUI=0"

SOURCES_SHARED = src/csv_data_manipulator.cpp src/fpga.cpp src/fine_grained.cpp
SOURCES_SHARED += without_gui/src/flora.cpp without_gui/src/main.cpp

with_partitioning: SOURCES = src/partition.cpp src/milp_model_zynq_with_partition.cpp
with_partitioning: BIN = run_with_partition
with_partitioning: CFLAGS += -DWITH_PARTITIONING
with_partitioning: build

without_partitioning: SOURCES = src/milp_model_zynq.cpp
without_partitioning: BIN = run_without_partition
without_partitioning: build

build:
	$(CC) -o $(BIN) $(CFLAGS) $(SOURCES_SHARED) $(SOURCES) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f *run	
