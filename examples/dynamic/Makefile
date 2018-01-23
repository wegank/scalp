
ScaLP_LIB_PATH=/Users/pluto/research/develop/inst//lib
ScaLP_INC_PATH=/Users/pluto/research/develop/inst//include
OTHER_FLAGS= -O2 # -D_GLIBCXX_USE_CXX11_ABI=0
CXX?=clang++


all: build run

build:
	$(CXX) dynamic.cc -std=c++11 $(OTHER_FLAGS) -o dynamic -I$(ScaLP_INC_PATH) -L$(ScaLP_LIB_PATH) -lScaLP -Wl,-rpath,$(ScaLP_LIB_PATH)

run:
	@echo -e ""
	./dynamic

clean:
	-rm dynamic dynamic.lp aa.lp 2> /dev/null || :
