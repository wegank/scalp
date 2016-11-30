
ILP_LIB_PATH=../../lib
ILP_INC_PATH=../../include
OTHER_FLAGS= -O2 # -D_GLIBCXX_USE_CXX11_ABI=0
CXX?=clang++


all: build run

build:
	$(CXX) dynamic.cc -std=c++11 $(OTHER_FLAGS) -o dynamic -I$(ILP_INC_PATH) -L$(ILP_LIB_PATH) -lILP -lILP-Dynamic -Wl,-rpath,$(ILP_LIB_PATH)

run:
	@echo -e ""
	./dynamic

clean:
	-rm dynamic dynamic.lp aa.lp 2> /dev/null || :
