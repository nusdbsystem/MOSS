# build with the Makefile under concurrent_runtime
USE_WORKER_SERVICE ?= 0
PROJECT_ROOT_DIR ?= $(shell readlink -f ../..)
INSTALL_PREFIX ?= $(shell readlink -f ..)

ifeq ($(USE_WORKER_SERVICE), 1)
	SGX_SDK ?= /opt/intel/sgxsdk
	SGX_MODE ?= HW
	SGX_ARCH ?= x64
	SGX_DEBUG ?= 1

	SGX_RA_TLS_DIR ?= $(PROJECT_ROOT_DIR)/deps/install/ratls
	UNTRUSTED_INFERENCE_RT ?= 1
	INFERENCE_RT_DIR ?= $(PROJECT_ROOT_DIR)/models/install
	WORKER_INSTALL_PATH ?= $(PROJECT_ROOT_DIR)/worker/install
endif # USE_WORKER_SERVICE

### Intel(R) SGX SDK Settings ###
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

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

APP_DCAP_LIBS := -lsgx_dcap_ql -lsgx_dcap_quoteverify
### Intel(R) SGX SDK Settings ###

COMMON_CXX_FLAGS := $(CONCURRENT_RT_CXX_FLAGS) -I$(PROJECT_ROOT_DIR)

.PHONY: all clean install

all: sample-server

Sources := main.cc
ifeq ($(USE_WORKER_SERVICE), 1)
	Sources += worker_service.cc
# setup compile flags
	COMMON_CXX_FLAGS += -DUSE_WORKER_SERVICE 
	COMMON_CXX_FLAGS += $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes
# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
				COMMON_CXX_FLAGS += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
				COMMON_CXX_FLAGS += -DNDEBUG -DEDEBUG -UDEBUG
else
				COMMON_CXX_FLAGS += -DNDEBUG -UEDEBUG -UDEBUG
endif # SGX_DEBUG
	COMMON_CXX_FLAGS += -I$(SGX_SDK)/include -I$(WORKER_INSTALL_PATH)/include
# setup linker flags
	COMMON_LINK_FLAGS += -L$(WORKER_INSTALL_PATH)/lib -l:libworker.a

	COMMON_LINK_FLAGS += $(CONCURRENT_RT_LINK_FLAGS)

	SGX_RA_TLS_LIB=$(SGX_RA_TLS_DIR)/lib
	COMMON_LINK_FLAGS += -L$(SGX_RA_TLS_LIB) -lratls_attester_u -lratls_challenger -lratls_common_u\
	-l:libcurl-wolfssl.a -l:libwolfssl.a \
	-L$(SGX_LIBRARY_PATH)	-l$(Urts_Library_Name) $(APP_DCAP_LIBS) \
	-lpthread -lz -lm -lcrypto
ifeq ($(UNTRUSTED_INFERENCE_RT), 1)
	COMMON_LINK_FLAGS += -L$(INFERENCE_RT_DIR)/lib -l:libuntrusted_inference_rt.a
endif # UNTRUSTED_INFERENCE_RT
ifneq ($(SGX_MODE), HW)
	COMMON_LINK_FLAGS += -lsgx_uae_service_sim
else
	COMMON_LINK_FLAGS += -lsgx_uae_service
endif
else # build default server
	COMMON_LINK_FLAGS := $(CONCURRENT_RT_LINK_FLAGS)
	COMMON_CXX_FLAGS += -g -O0 -Wall -Wextra -I$(PROJECT_ROOT_DIR)
	Sources += sample_service.cc
endif # USE_WORKER_SERVICE

Common_CXX_Flags += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls

Source_objects = $(Sources:.cc=.o)

%.o: %.cc
	$(CXX) $(COMMON_CXX_FLAGS) -c $< -o $@

ifneq ($(USE_WORKER_SERVICE), 1)
json.o: $(PROJECT_ROOT_DIR)/common/json.cc
	$(CXX) $(COMMON_CXX_FLAGS) -c $< -o $@

Source_objects += json.o

endif # build default server

sample-server: $(Source_objects)
	$(CXX) $^ -o $@ $(COMMON_LINK_FLAGS)

install:
	install -d $(INSTALL_PREFIX)/bin
	install -C -m 755 sample-server $(INSTALL_PREFIX)/bin/server
ifeq ($(USE_WORKER_SERVICE), 1)
	install -d $(INSTALL_PREFIX)/lib
	install -C -m 664 $(WORKER_INSTALL_PATH)/lib/Worker_Enclave.signed.so $(INSTALL_PREFIX)/lib
endif # USE_WORKER_SERVICE

clean:
	rm -f *.o
	rm -f sample-server
