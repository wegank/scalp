
ILP_LIB_PATH=../../lib
ILP_INC_PATH=../../include
OTHER_FLAGS= -O2
CXX?=clang++


all: build run

build:
	$(CXX) simple_gurobi.cc -std=c++11 $(OTHER_FLAGS) -o simple_gurobi -I$(ILP_INC_PATH) -L$(ILP_LIB_PATH) -lILP -lILP-Gurobi -Wl,-rpath,$(ILP_LIB_PATH)

run:
	@echo -e ""
	./simple_gurobi

clean:
	-rm simple_gurobi simple_gurobi.lp aa.lp 2> /dev/null || :
