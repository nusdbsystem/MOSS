# For tvm crt, we use the cmake files provided by tvm to install the sources files of standalone crt to the local deps directory. And build the dependencies libraries with enclave flags.
CRT_SRCS_LOCAL = $(abspath deps/standalone_crt)
### Intel(R) SGX SDK Settings ###
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 0
ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g -DSGX_DEBUG
else
        SGX_COMMON_CFLAGS += -O2
endif

SGX_COMMON_CFLAGS += -Wall -Wextra -Wchar-subscripts -Wno-coverage-mismatch \
										-Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
### Intel(R) SGX SDK Settings ###

### Project Settings ###
SGX_Include_Paths := -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc \
						 -I$(SGX_SDK)/include/libcxx

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        SGX_COMMON_CFLAGS += -DNDEBUG -DEDEBUG -UDEBUG
else
        SGX_COMMON_CFLAGS += -DNDEBUG -UEDEBUG -UDEBUG
endif

Enclave_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden \
	-fpie -fstack-protector -fno-builtin -fno-builtin-printf

CRT_SRCS_LOCAL = $(abspath deps/standalone_crt)
Enclave_C_Cpp_Flags += $(SGX_Include_Paths) -I./src -I$(CRT_SRCS_LOCAL)/include

Enclave_C_Flags := -Wno-implicit-function-declaration -std=c11 \
	-Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
Enclave_C_Flags += $(Enclave_C_Cpp_Flags)

Enclave_Cpp_Flags := -Wnon-virtual-dtor -std=c++11 -nostdinc++
Enclave_Cpp_Flags += $(Enclave_C_Cpp_Flags)

App_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes
App_C_Cpp_Flags += 	-Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
App_C_Cpp_Flags += -I./src -I$(SGX_SDK)/include
### Project Settings ###

### Phony targets ###-printf '%P\n'
.PHONY: checks all install-tflm-local tflm_libs clean
### Phony targets ###

### some checking before building any targets
checks:
ifndef PROJECT_ROOT_DIR
	$(error PROJECT_ROOT_DIR is not set. Please set to secure serverless inference project root directory)
endif

# pulling dependency for a certain model inference framework runtime
# makes building and installing customized model/input-dependent inference runtime easier
# DEP_LOCAL_INCLUDE_DIR := deps/include

### Install tvm crt to the current directory ###
deps:
	@echo Installing tvm crt sources locally
	mkdir -p build && cd build && cmake ../crt_install && make standalone_crt && cd ..
	mkdir -p deps && cp -r build/standalone_crt deps/ && cd deps/standalone_crt && patch -p1 < ../../crt_install/tvm-0.9.patch && patch -p1 < ../../crt_install/tvm-0.9-tls.patch && patch -p1 < ../../crt_install/min-asan.patch && patch -p1 < ../../crt_install/memset-bug-fix.patch && cd ../..
	rm -rf build
	@echo tvm crt sources headers installed
### Install tvm crt to the current directory ###

### Edger8r related sourcs ###
src/Enclave_u.h: $(SGX_EDGER8R) src/Enclave.edl
	@echo "execute $(SGX_EDGER8R) --header-only --untrusted src/Enclave.edl --untrusted-dir src"
	$(SGX_EDGER8R) --header-only --untrusted src/Enclave.edl --untrusted-dir src --search-path $(PROJECT_ROOT_DIR) --search-path $(PROJECT_ROOT_DIR)/models/include
	@echo "GEN => $@"
### Edger8r related sourcs ###

lib: 
	mkdir -p $@

src/ocall_patch.o: src/ocall_patch.cc src/Enclave_u.h
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CC <= $<" 

lib/libtvm_u.a: src/ocall_patch.o
	ar rcs $@ $<
	@echo "LINK => $@"

## build standalone crt libs 
deps/lib/libcommon.a: deps
	cd $(CRT_SRCS_LOCAL) && make QUIET= BUILD_DIR=$(abspath .)/deps/lib CRT_CONFIG=$(abspath src/crt_config/crt_config.h) "EXTRA_CFLAGS=$(Enclave_C_Flags)" common

deps/lib/libgraph_executor.a: deps
	cd $(CRT_SRCS_LOCAL) && make QUIET= BUILD_DIR=$(abspath .)/deps/lib CRT_CONFIG=$(abspath src/crt_config/crt_config.h) "EXTRA_CFLAGS=$(Enclave_C_Flags)" graph_executor
	
deps/lib/libmemory.a: deps
	cd $(CRT_SRCS_LOCAL) && make QUIET= BUILD_DIR=$(abspath .)/deps/lib CRT_CONFIG=$(abspath src/crt_config/crt_config.h) "EXTRA_CFLAGS=$(Enclave_C_Flags)" memory

deps/objs: deps/lib/libmemory.a deps/lib/libgraph_executor.a deps/lib/libcommon.a
	mkdir deps/objs
	find ./deps -type f -name "*.o" -exec cp -prv '{}' deps/objs ';'
## build standalone crt libs 

src/bundle_static.o: src/bundle_static.c deps
	$(CC) $(Enclave_C_Flags) -Isrc/crt_config -c $< -o $@
	@echo "CC   <=  $<"

Enclave_Objects := src/bundle_static.o 
Library_Deps := deps/lib/libcommon.a deps/lib/libgraph_executor.a deps/lib/libmemory.a

lib/libtvm_t.a: deps/objs $(Enclave_Objects)
	ar rcs $@ $(Enclave_Objects) $(wildcard deps/objs/*.o)
	@echo "LINK => $@"

tvm_libs: lib lib/libtvm_t.a lib/libtvm_u.a

### Build and install tflm into this directory ###

all: checks tvm_libs

clean:
	@rm -f src/*.o
	@rm -f src/Enclave_u.h
	@rm -rf lib

mrproper: clean
	@rm -rf deps
