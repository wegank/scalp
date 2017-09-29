
ScaLP_LIB_PATH=../../lib
ScaLP_INC_PATH=../../include
OTHER_FLAGS= -O2
CXX?=clang++


all: build run

build:
	$(CXX) advanced_gurobi.cc -std=c++11 $(OTHER_FLAGS) -o advanced_gurobi -I$(ScaLP_INC_PATH) -L$(ScaLP_LIB_PATH) -lScaLP -lScaLP-Gurobi -Wl,-rpath,$(ScaLP_LIB_PATH)

run:
	@echo -e ""
	./advanced_gurobi

clean:
	-rm advanced_gurobi advanced_gurobi.lp aa.lp 2> /dev/null || :
