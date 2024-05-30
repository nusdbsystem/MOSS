### Project Settings ###
PROJECT_ROOT_DIR ?= $(shell readlink -f ..)
SGX_RA_TLS_DIR ?= $(PROJECT_ROOT_DIR)/deps/install/ratls

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

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif

APP_DCAP_LIBS := -lsgx_dcap_ql -lsgx_dcap_quoteverify
### Intel(R) SGX SDK Settings ###

### Project Settings ###
Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes -I.
Common_C_Cpp_Flags += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
Common_C_Flags := -Wjump-misses-init -Wstrict-prototypes \
										-Wunsuffixed-float-constants
SGX_RA_TLS_Extra_Flags := -DWOLFSSL_SGX 


# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        Common_C_Cpp_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        Common_C_Cpp_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        Common_C_Cpp_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_C_Cpp_Flags := $(Common_C_Cpp_Flags) $(SGX_RA_TLS_Extra_Flags) -Iuntrusted -I$(SGX_SDK)/include -I$(PROJECT_ROOT_DIR) -I$(SGX_RA_TLS_DIR)/include
### Project Settings ###

### Linking setting ###
App_Link_Flags := $(SGX_COMMON_CFLAGS) \
	-L$(SGX_RA_TLS_DIR)/lib -lratls_attester_u -lratls_challenger -lratls_common_u\
	-L$(SGX_LIBRARY_PATH)	-l$(Urts_Library_Name) $(APP_DCAP_LIBS) \
	-L$(SGX_RA_TLS_DIR)/lib -l:libcurl-wolfssl.a -l:libwolfssl.a \
	-lpthread -lz -lm

## Add sgx_uae_service library to link ##
ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif
### Linking setting ###

### Phony targets ###
.PHONY: all clean

### Build all ###
ifeq ($(Build_Mode), HW_RELEASE)
all: key_server
	@echo "Build App [$(Build_Mode)|$(SGX_ARCH)] success!"
	@echo
	@echo "*********************************************************************************************************************************************************"
	@echo "PLEASE NOTE: In this mode, please sign the Server_Enclave.so first using Two Step Sign mechanism before you run the app to launch and access the enclave."
	@echo "*********************************************************************************************************************************************************"
	@echo

else
all: key_server
endif

### Sources ###
## Edger8r related sources ##
untrusted/Enclave_u.c: $(SGX_EDGER8R) trusted/Enclave.edl
	@echo Entering ./untrusted 
	cd ./untrusted && $(SGX_EDGER8R) --untrusted ../trusted/Enclave.edl --search-path ../trusted --search-path $(SGX_SDK)/include --search-path $(SGX_RA_TLS_DIR)/include
	@echo "GEN  =>  $@"

untrusted/Enclave_u.o: untrusted/Enclave_u.c
	$(CC) $(App_C_Cpp_Flags) $(Common_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"
## Edger8r related sources ##

## build files needed from other directory

untrusted/fileutil.o: ../common/fileutil.cc
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

untrusted/channel_server.o: ../message/channel_server.cc
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

untrusted/fs_store.o: ../store/fs_store.cc
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

untrusted/%.o: untrusted/%.cc
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

ifeq ($(SGX_DEBUG), 1)
untrusted/base64.o: ../common/base64.cc
	$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX   <=  $<"
endif # SGX_DEBUG

App_Objects := untrusted/fileutil.o untrusted/fs_store.o \
	untrusted/channel_server.o untrusted/key_server.o untrusted/ocall_patches.o
	
ifeq ($(SGX_DEBUG), 1)
App_Objects += untrusted/base64.o
App_Link_Flags += -lcrypto
endif # SGX_DEBUG

## Build server app ##
key_server: untrusted/Enclave_u.o $(App_Objects)
	$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"
### Sources ###

### Clean command ###
clean:
	@rm -f key_server $(App_Objects) untrusted/Enclave_u.* 
