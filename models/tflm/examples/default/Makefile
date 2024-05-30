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

SGX_COMMON_FLAGS += -Wall -Wextra -Wchar-subscripts -Wno-coverage-mismatch \
										-Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
### Intel(R) SGX SDK Settings ###

### Project Settings ###
SGX_Include_Paths := -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc \
						 -I$(SGX_SDK)/include/libcxx

Enclave_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) $(SGX_COMMON_FLAGS) -nostdinc \
	-fvisibility=hidden -fpie -fstack-protector -fno-builtin -fno-builtin-printf \

Enclave_C_Flags := -Wno-implicit-function-declaration -std=c11 \
	-Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
Enclave_Cpp_Flags := -Wnon-virtual-dtor -std=c++11 -nostdinc++

Enclave_C_Cpp_Flags += 	-I. $(SGX_Include_Paths) -I$(PROJECT_ROOT_DIR) \
	-I$(DEP_TFLM_DIR)/deps/include/ \
	-I$(DEP_TFLM_DIR)/src \
	-I$(DEP_TFLM_DIR)/deps/include/tflm \
	-I$(DEP_TFLM_DIR)/deps/include/flatbuffers \
	-I$(DEP_TFLM_DIR)/deps/include/gemmlowp \
	-I$(DEP_TFLM_DIR)/deps/include/ruy

Enclave_C_Cpp_Flags += -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -ffunction-sections -fdata-sections -fmessage-length=0 -DTF_LITE_STATIC_MEMORY -Wsign-compare -Wdouble-promotion -Wunused-variable -Wmissing-field-initializers -Wunused-function -Wswitch -Wstrict-aliasing -Wno-unused-parameter

### Project Settings ###

### Phony targets ###
.PHONY: check all clean

Trusted_Sources := $(wildcard ./*.cc)
Trusted_Objects := $(Trusted_Sources:.cc=.o)

%.o: %.cc
	@echo $(CXX) $(Enclave_C_Cpp_Flags) $(Enclave_Cpp_Flags) -c $< -o $@
	$(CXX) $(Enclave_C_Cpp_Flags) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

libtrusted_inference_rt.a: $(Trusted_Objects)
	@echo "Creating trusted part of inference runtime"
	cp $(DEP_TFLM_DIR)/lib/libtflm_t.a $@
	ar r $@ $(Trusted_Objects)
	@echo "Created trusted part of inference runtime"

libuntrusted_inference_rt.a:
	@echo "Creating untrusted part of inference runtime"
	cp $(DEP_TFLM_DIR)/lib/libtflm_u.a $@
	@echo "Created untrusted part of inference runtime"

check:
ifndef PROJECT_ROOT_DIR
	$(error PROJECT_ROOT_DIR is not set. Please set to secure serverless inference project root directory)
endif
ifndef DEP_TFLM_DIR
	$(error DEP_TFLM_DIR is not set. Please set to secure serverless inference project tflm dependency directory)
endif

all: check libtrusted_inference_rt.a libuntrusted_inference_rt.a
	@echo "Copy the library edl file"
	cp $(DEP_TFLM_DIR)/src/Enclave.edl ./Enclave.edl

INSTALL_PREFIX ?= ./install
install:
	install -d $(INSTALL_PREFIX)/lib
	cp libtrusted_inference_rt.a $(INSTALL_PREFIX)/lib
	cp libuntrusted_inference_rt.a $(INSTALL_PREFIX)/lib
	cp Enclave.edl $(INSTALL_PREFIX)/include/InferenceRT_Enclave.edl

clean:
	@rm -f *.o *.a *.edl
