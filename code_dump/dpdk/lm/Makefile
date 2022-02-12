AVIP_PLATFORM ?= PLT20
       
ifeq ($(AVIP_PLATFORM),PLT21)
override GCC_HOME = /grid/common/pkgs/gcc/v9.3.0p2
ABI_VAL=1
STD_VER?=c++17
else
override GCC_HOME = /grid/common/pkgs/gcc/v6.3.0
ABI_VAL=0
STD_VER?=c++14
endif

export PATH:=$(GCC_HOME)/bin:${PATH}
export LD_LIBRARY_PATH:=$(GCC_HOME)/lib64:${LD_LIBRARY_PATH}

default:clean
	@g++ 			\
		-m64 -s     \
		-std=$(STD_VER)	\
		-D_GLIBCXX_USE_CXX11_ABI=$(ABI_VAL)	\
		startvm.cpp \
		-o startvm

clean:
	@rm -rf startvm
