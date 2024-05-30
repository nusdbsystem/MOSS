# For TFLM, we only need to install the headers and copy over the static library
# The release mode static library can be directly linked with enclave codes
TFLM_LOCAL_PATH ?= $(PROJECT_ROOT_DIR)/deps/install/tflm
### Intel(R) SGX SDK Settings ###
SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1
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

SGX_COMMON_CFLAGS := $(SGX_COMMON_CFLAGS) 

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

Enclave_Cpp_Flags := $(Enclave_C_Cpp_Flags) \
	-Wnon-virtual-dtor -std=c++11 -nostdinc++
Enclave_Cpp_Flags += -I. -I./src $(SGX_Include_Paths) 

App_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes 
App_C_Cpp_Flags += -I. -I./src -I$(SGX_SDK)/include
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
DEP_LOCAL_INCLUDE_DIR := deps/include

### Install tflm to the current directory ###
install-tflm-local:
	@echo Installing tflm headers locally
	mkdir -p $(DEP_LOCAL_INCLUDE_DIR)
	cp -r $(TFLM_LOCAL_PATH)/include/* $(DEP_LOCAL_INCLUDE_DIR)
	@echo tflm headers installed
### Install tflm to the current directory ###

### Edger8r related sourcs ###
src/Enclave_u.h: $(SGX_EDGER8R) src/Enclave.edl
	$(SGX_EDGER8R) --header-only --untrusted src/Enclave.edl --untrusted-dir src --search-path $(PROJECT_ROOT_DIR) --search-path $(PROJECT_ROOT_DIR)/models/include
	@echo "GEN => $@"

src/Enclave_t.h: $(SGX_EDGER8R) src/Enclave.edl
	$(SGX_EDGER8R) --header-only --trusted src/Enclave.edl --trusted-dir src --search-path $(PROJECT_ROOT_DIR) --search-path $(PROJECT_ROOT_DIR)/models/include
	@echo "GEN => $@"
### Edger8r related sourcs ###

lib: 
	mkdir -p $@

src/io_patch_u.o: $(PROJECT_ROOT_DIR)/common/io_patch_u.cc src/Enclave_u.h
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX <= $<" 

lib/libtflm_u.a: src/io_patch_u.o
	ar rcs $@ $<
	@echo "LINK => $@"

src/io_patch_t.o: $(PROJECT_ROOT_DIR)/common/io_patch_t.cc src/Enclave_t.h
	$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX <= $<"

lib/libtflm_t.a: src/io_patch_t.o
	cp $(TFLM_LOCAL_PATH)/lib/libtflm_t.a lib/libtflm_t.a
	ar r $@ $<
	@echo "LINK => $@"

tflm_libs: lib lib/libtflm_t.a lib/libtflm_u.a

### Build and install tflm into this directory ###

all: checks install-tflm-local tflm_libs

clean:
	@rm -f src/Enclave_t.h
	@rm -f src/Enclave_u.h
	@rm -f src/*.o
	@rm -rf lib

mrproper: clean
	@rm -rf deps
