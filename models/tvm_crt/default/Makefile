MODULE_OBJS_DIR ?= $(DEP_TVMCRT_DIR)/module
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

Enclave_C_Cpp_Flags += $(SGX_Include_Paths) -I$(PROJECT_ROOT_DIR) \
	-I. -I$(DEP_TVMCRT_DIR) \
	-I$(DEP_TVMCRT_DIR)/deps/standalone_crt/include

Enclave_C_Cpp_Flags += -fno-exceptions -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -Wsign-compare -Wdouble-promotion -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wstrict-aliasing -Wno-unused-parameter

Enclave_Cpp_Flags := -Wnon-virtual-dtor -std=c++11 -nostdinc++

### Project Settings ###

## generated model artifacts
MODEL_OBJ = $(MODULE_OBJS_DIR)/model_c/devc.o $(MODULE_OBJS_DIR)/model_c/lib0.o $(MODULE_OBJS_DIR)/model_c/lib1.o
## generated model artifacts

### Phony targets ###
.PHONY: check all clean

Enclave.edl: $(DEP_TVMCRT_DIR)/src/Enclave.edl
	@echo "Copy the library edl file"
	cp $(DEP_TVMCRT_DIR)/src/Enclave.edl ./Enclave.edl

### Edger8r related sourcs ###
Enclave_t.h: $(SGX_EDGER8R) Enclave.edl
	$(SGX_EDGER8R) --header-only --trusted Enclave.edl --trusted-dir . --search-path $(PROJECT_ROOT_DIR) --search-path $(PROJECT_ROOT_DIR)/models/include
	@echo "GEN => $@"
### Edger8r related sourcs ###

tvm_default.o: tvm_default.cc Enclave_t.h
	$(CXX) $(Enclave_Cpp_Flags) $(Enclave_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

libtrusted_inference_rt.a: tvm_default.o ${MODEL_OBJ}
	@echo "Creating trusted part of inference runtime"
	cp $(DEP_TVMCRT_DIR)/lib/libtvm_t.a $@
	ar r $@ $^
	@echo "Created trusted part of inference runtime"

libuntrusted_inference_rt.a:
	@echo "Creating untrusted part of inference runtime"
	cp $(DEP_TVMCRT_DIR)/lib/libtvm_u.a $@
	@echo "Created untrusted part of inference runtime"

check:
ifndef PROJECT_ROOT_DIR
	$(error PROJECT_ROOT_DIR is not set. Please set to secure serverless inference project root directory)
endif
ifndef DEP_TVMCRT_DIR
	$(error DEP_TVMCRT_DIR is not set. Please set to secure serverless inference project tflm dependency directory)
endif

all: check libtrusted_inference_rt.a libuntrusted_inference_rt.a

INSTALL_PREFIX ?= ./install
install:
	install -d $(INSTALL_PREFIX)/lib
	cp libtrusted_inference_rt.a $(INSTALL_PREFIX)/lib
	cp libuntrusted_inference_rt.a $(INSTALL_PREFIX)/lib
	cp Enclave.edl $(INSTALL_PREFIX)/include/InferenceRT_Enclave.edl

clean:
	@rm -f *.o *.a *.edl
	@rm -f Enclave_t.h
